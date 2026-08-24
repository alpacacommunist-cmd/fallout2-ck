#include "ck_ids.h"
#include "ck_utils.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "object/critter/ck_critter.h"
#include "object/critter/ck_stats.h"
#include "ck_messages/ck_messages.h"

#include "proto_types.h"
#include "animation.h"

#include <cstring>
#include <unordered_set>

#include "ck_log.h"
static const Logger logger("CK Critter");

namespace ck::dispatcher {
    const char* current_mod_context();
}

namespace fallout {
	void _combat_ai(Object* a1, Object* a2);
	bool _combatai_want_to_join(Object* a1);
    void _combat_turn_run();

    int protoGetProto(int pid, fallout::Proto** proto);
	int proto_new(int* pid, fallout::ObjectType type);
	int proto_copy_proto(int srcPid, int dstPid);

    Object* objectFindFirstAtLocation(int elevation, int tile);
    Object* objectFindNextAtLocation();

	int mapGetCurrentMap();
}

namespace ck::critter {
    static std::unordered_set<int> g_custom_prototypes;
    static int g_next_spawn_index = 0;

    static std::string generate_unique_tag() {
        return "spawn_" + std::to_string(g_next_spawn_index++);
    }

    static bool has_proto_params(const CritterLuaProtoParams* params) {
        return !(utils::is_blank(params->name) && utils::is_blank(params->description));
    }

    static void reset_spawn_counter() {
        g_next_spawn_index = 0;
    }

    static void clear_custom_prototypes() {
        g_custom_prototypes.clear();
    }

    void clear_spawn_queues() {
        reset_spawn_counter();
        clear_custom_prototypes();
        logger.debug("Cleared map context critter queues");
    }

    bool has_custom_prototype(int pid) {
        return g_custom_prototypes.count(pid) > 0;
    }


    int allocate_unique_proto(int base_pid, const std::string& lua_tag, const CritterLuaProtoParams* params) {
        int unique_pid = 0;

        if (fallout::proto_new(&unique_pid, static_cast<fallout::ObjectType>(ck::ids::object_types::CRITTER)) != 0) {
            logger.error("Couldn't allocate new prototype for '{}'", lua_tag);
            return -1;
        }

        if (fallout::proto_copy_proto(base_pid, unique_pid) != 0) {
            logger.error("Couldn't copy prototype data for '{}'", lua_tag);
            return -1;
        }

        fallout::Proto* generic_proto = nullptr;
        if (fallout::protoGetProto(unique_pid, &generic_proto) == 0 && generic_proto) {
            fallout::CritterProto* critter_proto = reinterpret_cast<fallout::CritterProto*>(generic_proto);

            int clean_pid = ck::ids::clean_pid(unique_pid);

            int msg_name_id = clean_pid * 100;
            int msg_desc_id = msg_name_id + 1;

            critter_proto->messageId = msg_name_id;

            if (!utils::is_blank(params->name)) {
                ck::messages_add_string("pro_crit.msg", msg_name_id, params->name);
            }

            if (!utils::is_blank(params->description)) {
                ck::messages_add_string("pro_crit.msg", msg_desc_id, params->description);
            }
        } else {
            logger.error("Failed to get generic proto for allocated PID: {}", unique_pid);
            return -1;
        }

        logger.info("Created unique prototype for '{}' PID: {}", lua_tag, unique_pid);
        g_custom_prototypes.insert(unique_pid);
        return unique_pid;
    }

	static fallout::Object* create(int pid, int tile, int elevation) {
		fallout::Object* critter = ck_object_create(pid, tile, elevation, true);

		if (critter != nullptr) {
			return critter;
		}

		return nullptr;
	}

	CritterLua spawn(int pid, int tile, int elevation, const char* tag, const CritterLuaProtoParams* params) {
		int map_id          = fallout::mapGetCurrentMap();
        int source_pid      = pid;

        bool prototype_required = !utils::is_blank(tag) && has_proto_params(params);

        std::string lua_tag      = prototype_required ? std::string(tag) : generate_unique_tag();
		std::string_view mod_id  = dispatcher::current_mod_context();
		proxy::ObjectState state = proxy::get_object_state(map_id, lua_tag);

        CritterLua result{ -1, "" };

        if (state.elevation != -1) elevation = state.elevation;
		if (state.tile != -1)      tile = state.tile;

        // either alive or first spawn
        bool critter_alive = (state.hp > 0 || state.id == -1);

        if (critter_alive) {
            if (prototype_required) {
                int unique_pid = allocate_unique_proto(pid, lua_tag, params);
                if (unique_pid == -1) return result;

                pid = unique_pid;
            }

            fallout::Object* critter = create(pid, tile, elevation);
            if (critter == nullptr) return result;

            if (state.hp > 0) ck::critter_adjust_hp(critter, state.hp);

            LuaMeta meta = { mod_id, lua_tag, source_pid, critter->sid };
            result.lua_id = registry::created::add(critter, meta);

            critter->sid = ck::ids::make_sid_created(critter, result.lua_id);
            critter->data.critter.combat.team = 0;
        } else {
            fallout::Object* corpse = nullptr;
            fallout::Object* object = fallout::objectFindFirstAtLocation(elevation, tile);

            while (object != nullptr) {
                if (object->id == state.id) {
                    corpse = object;
                    break;
                }

                object = fallout::objectFindNextAtLocation();
            }

            if (corpse != nullptr) {
                result.lua_id = registry::modified::add(corpse, { mod_id, lua_tag, source_pid, -1 });
            }
        }

        utils::copy_to_buffer(result.lua_tag, sizeof(result.lua_tag), lua_tag);
        return result;
	}

	bool kill(int lua_id) {
		const CkCreatedObject* registry_object = ck::registry::created::get(lua_id);
		if (!registry_object || !registry_object->ptr) return false;

        // let fallout2-ce handle corpse
		registry_object->ptr->flags &= ~fallout::OBJECT_NO_SAVE;
        // store pid for proto messages
        registry_object->ptr->data.critter.radiation = registry_object->ptr->pid;
        // reset pid for savegames
		registry_object->ptr->pid = registry_object->meta.source_pid;

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

bool ck_critter_is_busy(void* ptr) {
	if (!ptr) return false; auto* obj = static_cast<fallout::Object*>(ptr);

	return fallout::animationIsBusy(obj) == -1;
}

int ck_critter_get_gender(void* ptr) {
	if (!ptr) return false; auto* obj = static_cast<fallout::Object*>(ptr);

    fallout::Gender gender = static_cast<fallout::Gender>(fallout::critterGetStat(obj, fallout::STAT_GENDER));
	return static_cast<int>(gender);
}

bool ck_critter_process_turn(void* ptr, int lua_id) {
	if (!ptr) return false; auto* critter = static_cast<fallout::Object*>(ptr);

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

