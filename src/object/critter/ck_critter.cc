#include "ck_ids.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "object/critter/ck_critter.h"
#include "object/critter/ck_stats.h"

#include "combat_defs.h"
#include "animation.h"

#include <cstring>

#include "ck_log.h"
static const Logger logger("CK Critter");

static int allocate_unique_proto(int base_pid, const std::string& lua_tag) {
	int unique_pid = 0;

	if (fallout::proto_new(&unique_pid, static_cast<fallout::ObjectType>(ck::ids::object_types::CRITTER)) != 0) {
		logger.error("Couldn't allocate new prototype for '{}'", lua_tag);
		return -1;
	}

	if (fallout::proto_copy_proto(base_pid, unique_pid) != 0) {
		logger.error("Couldn't copy prototype data for '{}'", lua_tag);
		return -1;
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

	CritterLua register_critter(int pid, int tile, const char* tag) {
		int map_id          = fallout::mapGetCurrentMap();

		std::string mod_id  = ck_get_current_mod_id();
		std::string lua_tag = (tag != nullptr ? std::string(tag) : std::string());
		ck::proxy::ObjectState state = ck::proxy::get_object_state(map_id, lua_tag);

		if (state.tile != -1) tile = state.tile;

		int source_pid = pid;

		if (!lua_tag.empty()) {
			int unique_pid = allocate_unique_proto(pid, lua_tag);
			if (unique_pid == -1) return { -1, ck_get_current_mod_id() };

			pid = unique_pid;
		}

		fallout::Object* critter = create_critter(pid, tile);
		if (critter == nullptr) return { -1, ck_get_current_mod_id() };

		if (state.hp > 0) ck::critter_adjust_hp(critter, state.hp);

		int lua_id = -1;
		LuaMeta meta = { mod_id, lua_tag, source_pid, critter->sid };

		lua_id = ck::registry::created::add(critter, meta);

		critter->sid = ck::ids::make_sid_created(critter, lua_id);
		critter->data.critter.combat.team = 0;

		return { lua_id, ck_get_current_mod_id() };
	}

	bool critter_kill(int lua_id) {
		const CkCreatedObject* object = ck::registry::created::get(lua_id);
		if (!object || !object->ptr) return false;

		// object->ptr->pid = object->meta.source_pid;
		// object->ptr->flags &= ~fallout::OBJECT_NO_SAVE;
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

CritterLua ck_critter_register(int pid, int tile, const char* tag) {
	return ck::register_critter(pid, tile, tag);
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

