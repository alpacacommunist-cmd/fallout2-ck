#include "utils/fallout/types.h"
#include "ck_ids.h"
#include "utils.h"

#include "lua/proxy/state.h"
#include "ck_registry/ck_registry.h"
#include "object/ck_object.h"
#include "object/critter/ck_critter.h"
#include "ck_proto/registry/ck_proto_registry.h"
#include "script/ck_script.h"

#include "ck_gameplay_common.h"
#include "runtime/mods.h"

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

    static void handle_dead_critter_spawn(int id, int pid, int tile, int elevation = fallout::gElevation) {
        fallout::Object* corpse = ck::object::find_id_at(tile, elevation, id);

        // Mod specifies custom name/description (for look_at/examine).
        // Assign allocated pid to cid to enable custom messages (temp workaround)
        // critter.cc (critterGetName(Object* obj)
        if (corpse != nullptr) {
            logger.debug("Critter {} is dead, corpse found (pid is written to object->`cid`)", pid);

            corpse->cid = pid;
        } else {
            // TODO: remove from state.db (if respawn)
            logger.debug("Removing respawning critter from state.db: {} (dead, corpse not found)", pid);
        }
    }

    void assign_script(fallout::Object* critter, int script_index) {
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

    fallout::Object* spawn(int pid, int tile, CritterLuaSpawnParams* spawn_params) {
        std::string lua_tag = spawn_params->tag;

        int map_id = common::current_map_id();
        const char* mod_id = mods::current_mod_id();

        // Check if state json exists for given tag
		proxy::ObjectState state = proxy::get_object_state(map_id, lua_tag);
        if (state.elevation != -1) spawn_params->elevation = state.elevation;
        if (state.tile != -1) tile = state.tile;

        fallout::Object* critter = create(pid, tile, spawn_params->elevation);
        if (critter == nullptr) return nullptr;

        if (state.hp > 0) ck::critter_adjust_hp(critter, state.hp);

        int critter_id = registry::created::add(critter, lua_tag.data(), mod_id);
        if (critter_id == -1) return nullptr;

        ck::critter::assign_script(critter, spawn_params->script_index);

        if (spawn_params->team != -1) {
            critter->data.critter.combat.team = spawn_params->team;
            logger.debug("Assigned team ID: {} to critter {}", spawn_params->team, lua_tag);
        }

        return critter;
	}

	bool kill(fallout::Object* object) {
		const auto* registry_object = ck::registry::get(object);
		if (!registry_object || !registry_object->object) return false;

        const Proto* critter_proto = ck::proto::registry::get(ck::ids::clean_pid(object->pid));

        // let fallout2-ce handle the corpse
		object->flags &= ~fallout::ObjectFlags::OBJECT_NO_SAVE;

        if (critter_proto) {
            logger.debug("Killed critter {} identified as custom prototype", critter_proto->pid);
            // store pid for proto messages
            object->cid = critter_proto->packed_pid();
            // reset pid for savegames
            object->pid = ck::ids::make_packed_pid(critter_proto->source_pid, critter_proto->object_type);

            logger.debug("OBJECT_NO_SAVE removed, PID set to: {}, CUSTOM pid saved to `cid`: {}",
                         object->pid, object->cid);
        }

		return true;
	}
}

fallout::Object* ck_critter_spawn(int pid, int tile, CritterLuaSpawnParams* spawn_params) {
	return ck::critter::spawn(pid, tile, spawn_params);
}

int ck_anim_begin(void* ptr, int request_options) {
	return fallout::reg_anim_begin(static_cast<fallout::AnimationRequestOptions>(request_options));
}

int ck_take_out_weapon(fallout::Object* critter) {
    CK_ENSURE_VALID_OBJECT(critter);

    fallout::Object* weapon = fallout::_ai_search_inven_weap(critter, false, nullptr);
    if (!weapon) return -1;

    fallout::WeaponAnimation weapon_anim_code = fallout::weaponGetAnimationCode(weapon);
    return fallout::inventoryEquipFunc(critter, weapon, fallout::Hand::HAND_RIGHT, false);
}

int ck_critter_anim_move_to(fallout::Object* critter, int tile, int elevation, int delay) {
    CK_ENSURE_VALID_OBJECT(critter);
	return fallout::animationRegisterMoveToTile(critter, tile, ck::utils::types::map_elevation(elevation), -1, delay);
}

int ck_critter_anim_play(fallout::Object* critter, int anim_id, int delay) {
    CK_ENSURE_VALID_OBJECT(critter);
	return fallout::animationRegisterAnimate(critter, static_cast<fallout::AnimationType>(anim_id), delay);
}

int ck_critter_anim_clear(fallout::Object* critter) {
    CK_ENSURE_VALID_OBJECT(critter);
	return fallout::reg_anim_clear(critter);
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

bool ck_critter_kill(fallout::Object* object) {
    if (object == nullptr) return false;
	return ck::critter::kill(object);
}

void ck_critter_handle_dead_critter_spawn(int id, int pid, int tile) {
    ck::critter::handle_dead_critter_spawn(id, pid, tile);
}
