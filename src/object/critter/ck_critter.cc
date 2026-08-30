#include "ck_ids.h"
#include "ck_utils.h"

#include "ck_lua_proxy/ck_lua_proxy_state.h"
#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "object/critter/ck_critter.h"
#include "object/critter/ck_critter_proto.h"

#include "animation.h"
#include "scripts.h"

#include <cstring>

#include "ck_log.h"
static const Logger logger("CK Critter");

namespace ck {
    namespace dispatcher {
        const char* current_mod_context();
    }

    namespace common {
        bool currently_in_combat();
    }

    // ck_stats.cc
	int critter_adjust_hp(fallout::Object* critter, int target_hp);
}

namespace fallout {
    // enum Hand : int { HAND_LEFT, HAND_RIGHT, HAND_COUNT };

    extern Object* gDude;
    int scriptAdd(int* sidPtr, int scriptType);

	void _combat_ai(Object* a1, Object* a2);
    Object* _ai_search_inven_weap(Object* critter, bool checkRequiredActionPoints, Object* defender);
	bool _combatai_want_to_join(Object* a1);
    void _combat_turn_run();

    Object* objectFindFirstAtLocation(int elevation, int tile);
    Object* objectFindNextAtLocation();

	int mapGetCurrentMap();
    int inventoryEquip(Object* critter, Object* item, Hand hand);
    WeaponAnimation weaponGetAnimationCode(Object* weapon);
}

namespace ck::critter {
    static std::unordered_map<std::string, int> g_mod_spawn_counters;

    static std::string generate_unique_tag(const std::string& mod_id) {
        int& index = g_mod_spawn_counters[mod_id];
        return "spawn_" + mod_id + "_" + std::to_string(index++);
    }

    static bool has_proto_params(const CritterLuaProtoParams* params) {
        return !(utils::is_blank(params->name) && utils::is_blank(params->description));
    }

    static void reset_spawn_counters() {
        g_mod_spawn_counters.clear();
    }

    void reset_spawn_counter_for_mod(const std::string& mod_id) {
        // clears spawn counters (e.g. spawn_mod_id_0, spawn_mod_id_1..)
        g_mod_spawn_counters.erase(mod_id);
        // clears mod-registered critter prototypes
        ck::critter::proto::clear_prototypes_for_mod(mod_id);

        logger.debug("Reset spawn counter for mod: {}", mod_id);
    }

    void clear_spawn_queues() {
        ck::critter::reset_spawn_counters();
        ck::critter::proto::clear_prototypes();

        logger.debug("Cleared map context critter queues");
    }

	static fallout::Object* create(int pid, int tile, int elevation) {
		fallout::Object* critter = ck_object_create(pid, tile, elevation, true);

		if (critter != nullptr) {
			return critter;
		}

		return nullptr;
	}

	CritterLua spawn(int pid, int tile, int elevation, const char* tag, const CritterLuaProtoParams* params) {
        CritterLua result{ -1, "" };

		int map_id         = fallout::mapGetCurrentMap();
		std::string mod_id = dispatcher::current_mod_context();

        int source_pid = pid;
        bool prototype_required = !utils::is_blank(tag) && has_proto_params(params);

        std::string lua_tag;
        if (prototype_required) {
            // Lua tag is passed explicitly
            lua_tag = std::string(tag);
            g_mod_spawn_counters[mod_id]++;
        } else {
            // Autogenerate tag
            lua_tag = generate_unique_tag(mod_id);
        }
        utils::copy_to_buffer(result.lua_tag, sizeof(result.lua_tag), lua_tag);

        // Check if state json exists for given tag
		proxy::ObjectState state = proxy::get_object_state(map_id, lua_tag);
        if (state.elevation != -1) elevation = state.elevation;
		if (state.tile != -1)      tile = state.tile;

        // either alive or first spawn
        bool critter_alive = (state.hp > 0 || state.id == -1);

        if (critter_alive) {
            if (prototype_required) {
                int unique_pid = ck::critter::proto::allocate(pid, params);
                if (unique_pid == -1) return result;

                pid = unique_pid;
            }

            fallout::Object* critter = create(pid, tile, elevation);
            if (critter == nullptr) return result;

            if (state.hp > 0) ck::critter_adjust_hp(critter, state.hp);

            LuaMeta meta = { mod_id, lua_tag, source_pid, critter->sid };
            result.lua_id = registry::created::add(critter, std::move(meta));

            // int proto_sid = ck::ids::clean_sid(ck::critter::proto::proto_sid_of(pid));
            if (critter->scriptIndex != -1) {
                logger.debug("PID {} uses native engine script slot {}", critter->pid, critter->scriptIndex);

                // logger.info("script remove {}", ck::ids::clean_sid(critter->sid));
                // fallout::scriptRemove(critter->sid);
                // critter->sid = -1;
                //
                // fallout::scriptAdd(&(critter->sid), fallout::SCRIPT_TYPE_CRITTER);
                fallout::Script* script = nullptr;

                if (fallout::scriptGetScript(critter->sid, &script) != -1) {
                    // script->index   = critter->scriptIndex;
                    script->ownerId = critter->id;
                    script->owner   = critter;

                    logger.info("Using script index: {}", critter->scriptIndex);
                    logger.info("Unpacked sid: {}", ck::ids::clean_sid(critter->sid));
                    logger.info("script owner id: {}", script->ownerId);

                    // script->flags |= (SCRIPT_FLAG_LOADED | SCRIPT_FLAG_EXECUTED);
                    logger.info("script flags: {}", script->flags);
                    fallout::_scr_find_str_run_info(critter->scriptIndex, &(script->field_50), critter->sid);

                    if (ck::common::currently_in_combat()) {
                        fallout::_scripts_clear_combat_requests(script);
                        fallout::scriptExecProc(critter->sid, fallout::SCRIPT_PROC_COMBAT);
                        logger.info("COMBAT");
                    } else {
                        fallout::scriptExecProc(critter->sid, fallout::SCRIPT_PROC_START);
                        fallout::scriptExecProc(critter->sid, fallout::SCRIPT_PROC_MAP_ENTER);
                        logger.info("NOT COMBAT");
                    }
                }
            } else {
                critter->sid = ids::make_sid_created(critter, result.lua_id);
                logger.debug("assigning lua SID {} to PID {}", critter->sid, critter->pid);
            }

            if (params->team != -1) {
                critter->data.critter.combat.team = params->team;
                logger.debug("Assigned team ID: {} to critter {}", params->team, lua_tag);
            }
        } else {
            // Has no custom proto attributes, body is handled by fallout2-ce
            if (!prototype_required) {
                result.lua_id = -2; return result;
            }

            fallout::Object* corpse = nullptr;
            fallout::Object* object = fallout::objectFindFirstAtLocation(elevation, tile);

            while (object != nullptr) {
                if (object->id == state.id) {
                    corpse = object;
                    break;
                }

                object = fallout::objectFindNextAtLocation();
            }

            // Mod specifies custom name/description (for look_at/examine). Assign custom SID to a corpse
            // to let lua handle procs
            if (corpse != nullptr) {
                result.lua_id = registry::modified::add(corpse, { mod_id, lua_tag, source_pid, -1 });
            }
        }

        return result;
	}

	bool kill(int lua_id) {
		const CkCreatedObject* registry_object = ck::registry::created::get(lua_id);
		if (!registry_object || !registry_object->ptr) return false;

        logger.info("critter killed! {}", lua_id);

        // let fallout2-ce handle the corpse
		registry_object->ptr->flags &= ~fallout::OBJECT_NO_SAVE;

        if (ck::critter::proto::has_custom_prototype(registry_object->ptr->pid)) {
            logger.debug("Killed critter {} identified as custom prototype", registry_object->ptr->pid);
            // store pid for proto messages
            registry_object->ptr->data.critter.radiation = registry_object->ptr->pid;
            // reset pid for savegames
            registry_object->ptr->pid = registry_object->meta.source_pid;
        }

        // std::string mod_id = std::move(registry_object->meta.mod_id), lua_tag = std::move(registry_object->meta.tag);
        // int source_pid     = registry_object->meta.source_pid;
        // fallout::Object* corpse = registry_object->ptr;
        //
        // ck::registry::created::remove_by_ptr(registry_object->ptr);
        // int modified_lua_id = ck::registry::modified::add(corpse, { std::move(mod_id), std::move(lua_tag), source_pid, -1 });

		return true;
	}
}

CritterLua ck_critter_spawn(int pid, int tile, int elevation, const char* tag, const CritterLuaProtoParams* params) {
	return ck::critter::spawn(pid, tile, elevation, tag, params);
}

int ck_anim_begin(void* ptr, int request_options) {
	return fallout::reg_anim_begin(static_cast<fallout::AnimationRequestOptions>(request_options));
}

int ck_anim_take_out_weapon(fallout::Object* critter, int delay) {
    CK_ENSURE_VALID_OBJECT(critter);

    fallout::Object* weapon = fallout::_ai_search_inven_weap(critter, false, fallout::gDude);
    if (!weapon) return -1;

    fallout::WeaponAnimation weapon_anim_code = fallout::weaponGetAnimationCode(weapon);
    return animationRegisterTakeOutWeapon(critter, weapon_anim_code, delay);
}

int ck_anim_move_to(void* ptr, int tile, int elevation) {
	if (!ptr) return -1; auto* object = static_cast<fallout::Object*>(ptr);

	return fallout::animationRegisterMoveToTile(object, tile, elevation, -1, 0);
}

int ck_anim_play(void* ptr, int anim_id) {
	if (!ptr) return -1; auto* object = static_cast<fallout::Object*>(ptr);

	return fallout::animationRegisterAnimate(object, static_cast<fallout::AnimationType>(anim_id), 0);
}

int ck_anim_clear(void* ptr) {
	if (!ptr) return -1; auto* object = static_cast<fallout::Object*>(ptr);
	return fallout::reg_anim_clear(object);
}

int ck_anim_end() {
	return fallout::reg_anim_end();
}

bool ck_critter_is_busy(fallout::Object* critter) {
    CK_ENSURE_VALID_OBJECT(critter);

	return fallout::animationIsBusy(critter) == -1;
}

bool ck_critter_process_turn(fallout::Object* critter, int lua_id) {
    CK_ENSURE_VALID_OBJECT(critter);

	if (critter->data.critter.combat.ap <= 0) {
		logger.info("combat_turn_run for {}", lua_id);
		fallout::_combat_turn_run();
		return true;
	}

	if (fallout::_combatai_want_to_join(critter)) {
		fallout::_combat_ai(critter, nullptr);
	} else {
		critter->data.critter.combat.ap = 0;
		fallout::_combat_turn_run();
	}

	return true;
}

bool ck_critter_kill(int lua_id) {
	return ck::critter::kill(lua_id);
}

void ck_critter_reset_spawn_counters_for_mod(const char* mod_id) {
    ck::critter::reset_spawn_counter_for_mod(std::string(mod_id));
}
