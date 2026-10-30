#include "utils/fallout/types.h"
#include "ck_ids.h"
#include "utils.h"

#include "ck_lua_proxy/ck_lua_proxy_state.h"
#include "ck_registry/ck_registry.h"
#include "object/ck_object.h"
#include "object/critter/ck_critter.h"
#include "ck_proto/registry/ck_proto_registry.h"
#include "script/ck_script.h"

#include "ck_gameplay_common.h"
#include "ck_mods/ck_mods.h"

#include "fallout2-ce/animation.h"

#include <cstring>

#include "log.h"
static constexpr logging::Context logger{"ck_critter.cc"};

namespace ck {
    // ck_stats.cc
	int critter_adjust_hp(fallout::Object* critter, int target_hp);
}

namespace fallout {
    extern Object* gDude;

    enum Hand : int { HAND_LEFT, HAND_RIGHT, HAND_COUNT, };
    int inventoryEquipFunc(Object* critter, Object* item, Hand hand, bool animate);

	void _combat_ai(Object* a1, Object* a2);
    Object* _ai_search_inven_weap(Object* critter, bool checkRequiredActionPoints, Object* defender);
	bool _combatai_want_to_join(Object* a1);
    void _combat_turn_run();

    Object* objectFindFirstAtLocation(int elevation, int tile);
    Object* objectFindNextAtLocation();

    WeaponAnimation weaponGetAnimationCode(Object* weapon);
}

namespace ck::critter {
	static fallout::Object* create(int pid, int tile, int elevation) {
		fallout::Object* critter = ck_object_create(pid, tile, elevation, true);

		if (critter != nullptr) {
            Proto* proto = ck::proto::registry::get(pid);
            if (proto) {
                critter->scriptIndex = proto->get_sid();
            }

            return critter;
        }

		return nullptr;
	}

    static int handle_dead_critter_spawn(int id, int pid, int tile, int elevation = fallout::gElevation) {
        fallout::Object* corpse = ck::object::find_id_at(tile, elevation, id);

        // Mod specifies custom name/description (for look_at/examine).
        // Assign allocated pid to corpse.radiation to enable custom messages (temp workaround)
        // critter.cc (critterGetName(Object* obj)
        if (corpse != nullptr) {
            logger.debug("Critter {} is dead, corpse found", pid);
            corpse->data.critter.radiation = pid;
        } else {
            // TODO: remove from state.db
            logger.debug("Removing critter from state.db: {} (dead, corpse not found)", pid);
        }

        return -3;
    }

    void assign_script(fallout::Object* critter, int script_index, int lua_id) {
        // Script index explicitly passed from LUA
        if (script_index != -1) {
            ck::script::assign_script_index_to_object(script_index, critter);
            ck::script::disable_map_updates_for_object(critter);

            logger.info("PID {} uses script_index: {} (passed explicitly)", critter->pid, script_index);
            return;
        }

        // Critter has PROTOTYPE script
        if (critter->scriptIndex != -1) {
            ck::script::assign_proto_script(critter);
            ck::script::disable_map_updates_for_object(critter);

            logger.info("PID {} uses script_index: {} (proto)", critter->pid, critter->scriptIndex);
            return;
        }

        // Otherwise assign custom script
        int custom_script_index = ck::ids::next_script_index(ck::mods::current_mod_id(), ck::ids::object_type(critter->pid));
        if (custom_script_index == -1) {
            return;
        } else {
            ck::script::assign_script_index_to_object(custom_script_index, critter);
            logger.info("PID {} uses script_index: {} (custom)", critter->pid, critter->scriptIndex);
        }
    }

	int spawn(int pid, int tile, CritterLuaSpawnParams* spawn_params) {
        int lua_id = -1;
        std::string lua_tag = spawn_params->tag;

        int map_id = common::current_map_id();
        const char* mod_id = mods::current_mod_id();

        // Locate source pid
        // It's being used when critter dies: source_pid is restored and
        // OBJECT_NO_SAVE is removed, body is handled by engine
        int source_pid = proto::registry::source_pid(pid);

        // Check if state json exists for given tag
		proxy::ObjectState state = proxy::get_object_state(map_id, lua_tag);
        if (state.elevation != -1) spawn_params->elevation = state.elevation;
        if (state.tile != -1) tile = state.tile;

        // Either alive or first spawn
        bool critter_alive = (state.hp > 0 || state.id == -1);

        if (!critter_alive) {
            return handle_dead_critter_spawn(state.id, pid, tile, spawn_params->elevation);
        }

        fallout::Object* critter = create(pid, tile, spawn_params->elevation);
        if (critter == nullptr) {
            return -1;
        }

        if (state.hp > 0) ck::critter_adjust_hp(critter, state.hp);

        LuaMeta meta = { mod_id, lua_tag, source_pid, critter->sid };
        lua_id = registry::created::add(critter, std::move(meta));

        ck::critter::assign_script(critter, spawn_params->script_index, lua_id);

        if (spawn_params->team != -1) {
            critter->data.critter.combat.team = spawn_params->team;
            logger.debug("Assigned team ID: {} to critter {}", spawn_params->team, lua_tag);
        }

        return lua_id;
	}

	bool kill(int lua_id) {
		const CkCreatedObject* registry_object = ck::registry::created::get(lua_id);
		if (!registry_object || !registry_object->ptr) return false;

        const Proto* critter_proto = ck::proto::registry::get(ck::ids::clean_pid(registry_object->ptr->pid));

        // let fallout2-ce handle the corpse
		registry_object->ptr->flags &= ~fallout::ObjectFlags::OBJECT_NO_SAVE;

        if (critter_proto) {
            logger.debug("Killed critter {} identified as custom prototype", critter_proto->pid);
            // store pid for proto messages
            registry_object->ptr->data.critter.radiation = critter_proto->packed_pid();
            // reset pid for savegames
            registry_object->ptr->pid = ck::ids::make_packed_pid(critter_proto->source_pid, critter_proto->object_type);

            logger.debug("OBJECT_NO_SAVE removed, PID set to: {}, CUSTOM pid saved to data.critter.radiation: {}",
                         registry_object->ptr->pid, registry_object->ptr->data.critter.radiation);
        }

		return true;
	}
}

int ck_critter_spawn(int pid, int tile, CritterLuaSpawnParams* spawn_params) {
	return ck::critter::spawn(pid, tile, spawn_params);
}

int ck_anim_begin(void* ptr, int request_options) {
	return fallout::reg_anim_begin(static_cast<fallout::AnimationRequestOptions>(request_options));
}

int ck_anim_take_out_weapon(fallout::Object* critter, int delay) {
    CK_ENSURE_VALID_OBJECT(critter);

    fallout::Object* weapon = fallout::_ai_search_inven_weap(critter, false, nullptr);
    if (!weapon) return -1;

    fallout::WeaponAnimation weapon_anim_code = fallout::weaponGetAnimationCode(weapon);
    return fallout::inventoryEquipFunc(critter, weapon, fallout::Hand::HAND_RIGHT, false);
}

int ck_anim_move_to(void* ptr, int tile, int elevation) {
	if (!ptr) return -1; auto* object = static_cast<fallout::Object*>(ptr);

	return fallout::animationRegisterMoveToTile(object, tile, ck::utils::types::map_elevation(elevation), -1, 0);
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
