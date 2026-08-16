#include "ck_ids.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "object/critter/ck_critter.h"
#include "object/critter/ck_stats.h"
#include "ck_messages/ck_messages.h"

#include "combat_defs.h"
#include "animation.h"

#include <cstring>

#include "ck_log.h"
static const Logger logger("CK Critter");

static int allocate_unique_proto(int base_pid, const std::string& lua_tag, const CritterLuaProtoParams* params) {
	int unique_pid = 0;

	if (fallout::proto_new(&unique_pid, static_cast<fallout::ObjectType>(ck::ids::object_types::CRITTER)) != 0) {
		logger.error("Couldn't allocate new prototype for '{}'", lua_tag);
		return -1;
	}

	if (fallout::proto_copy_proto(base_pid, unique_pid) != 0) {
		logger.error("Couldn't copy prototype data for '{}'", lua_tag);
		return -1;
	}

    int msg_name_id = unique_pid * 100;
    int msg_desc_id = msg_name_id + 1;

    if (params->name) {
        std::string_view name(params->name);
        if (!name.empty()) ck::messages_add_string("pro_crit.msg", msg_name_id, params->name);
    }

    if (params->description) {
        std::string_view description(params->description);
        if (!description.empty()) ck::messages_add_string("pro_crit.msg", msg_desc_id, params->description);
    }

	logger.info("Created unique prototype for '{}' PID: {}", lua_tag, unique_pid);
	return unique_pid;
}

namespace ck {
	fallout::Object* create_critter(int pid, int tile) {
		fallout::Object* critter = ck_object_create(pid, tile, true);

		if (critter != nullptr) {
			return critter;
		}

		return nullptr;
	}

	CritterLua register_critter(int pid, int tile, const char* tag, const CritterLuaProtoParams* params) {
		int map_id          = fallout::mapGetCurrentMap();
        int source_pid      = pid;
        int lua_id          = 1;

		std::string mod_id  = ck_get_current_mod_id();
		std::string lua_tag = (tag != nullptr ? std::string(tag) : std::string());
		ck::proxy::ObjectState state = ck::proxy::get_object_state(map_id, lua_tag);

		if (state.tile != -1) tile = state.tile;

        if (state.hp > 0 || state.id == -1) { // either alive or first spawn
            if (!lua_tag.empty()) {
                int unique_pid = allocate_unique_proto(pid, lua_tag, params);
                if (unique_pid == -1) return { -1, ck_get_current_mod_id() };

                pid = unique_pid;
            }

            fallout::Object* critter = create_critter(pid, tile);
            if (critter == nullptr) return { -1, ck_get_current_mod_id() };

            if (state.hp > 0) ck::critter_adjust_hp(critter, state.hp);

            LuaMeta meta = { mod_id, lua_tag, source_pid, critter->sid };
            lua_id = ck::registry::created::add(critter, meta);

            critter->sid = ck::ids::make_sid_created(critter, lua_id);
            critter->data.critter.combat.team = 0;

        } else {
            fallout::Object* corpse;
            fallout::Object* object = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);

            while (object != nullptr) {
                if (object->id == state.id) {
                    corpse = object;
                    break;
                }

                object = fallout::objectFindNextAtLocation();
            }

            if (corpse != nullptr) {
                lua_id = ck::registry::modified::add(corpse, { mod_id, lua_tag, source_pid, -1 });
            }
        }

        return { lua_id, ck_get_current_mod_id() };
	}

	bool critter_kill(int lua_id) {
		const CkCreatedObject* object = ck::registry::created::get(lua_id);
		if (!object || !object->ptr) return false;

		object->ptr->pid    = object->meta.source_pid;
        object->ptr->sid    = -1;
		object->ptr->flags &= ~fallout::OBJECT_NO_SAVE;

        ck::registry::created::remove_by_ptr(object->ptr);
        // ck::registry::modified::add(corpse, { mod_id, lua_tag, source_pid, -1 });
		// _combat_delete_critter(object->ptr);
		//
		// if (fallout::gDude->data.critter.combat.whoHitMe == object->ptr) {
		// 	fallout::gDude->data.critter.combat.whoHitMe = nullptr;
		// }

		return true;
	}
}


bool ck_in_combat() {
	return (fallout::gCombatState & fallout::COMBAT_STATE_IN_COMBAT) != 0;
}

CritterLua ck_critter_register(int pid, int tile, const char* tag, const CritterLuaProtoParams* params) {
	return ck::register_critter(pid, tile, tag, params);
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
	return ck::critter_kill(lua_id);
}

