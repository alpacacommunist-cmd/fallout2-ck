#include "ck_ids.h"
#include "object/ck_object.h"
#include "object/ck_object_registry.h"
#include "object/ck_critter.h"

#include "animation.h"
#include "stat.h"

#include <iostream>
#include <ostream>

namespace ck {

	int DEFAULT_CRITTER_SID = 13;

	fallout::Object* create_critter(int pid, int tile) {
		fallout::Object* critter = ck_object_create(pid, tile);

		if (critter != nullptr) {
			critter->flags |= fallout::OBJECT_NO_SAVE;
			critter->sid    = ck::make_full_sid(fallout::SCRIPT_TYPE_CRITTER, DEFAULT_CRITTER_SID);

			return critter;
		}

		return nullptr;
	}

	int register_critter(int pid, int tile) {
		fallout::Object* critter = create_critter(pid, tile);

		int lua_id    = -1;
		LuaCritterMeta meta = { critter->sid };

		if (critter != nullptr) {
			lua_id = gObjectRegistry.add(critter, meta);

			int custom_sid = ck::make_sid(lua_id);
			critter->sid   = ck::make_full_sid(fallout::SCRIPT_TYPE_CRITTER, custom_sid);
		}

		if (fallout::reg_anim_begin(0) == 0) {
			fallout::animationRegisterMoveToTile(critter, 20913, critter->elevation, -1, 0);
			fallout::reg_anim_end();
		} else {
			std::cout << "[CK Debug] FAILED to register animation" << std::endl;
		}

		return lua_id;
	}

	int critter_stat(fallout::Object* critter, int stat) {
		return fallout::critterGetStat(critter, stat);
	}

	int critter_pc_stat(int stat) {
		return fallout::pcGetStat(stat);
	}

}

