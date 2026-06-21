#include "ck_encoding.h"
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

void ck_critter_float_msg(int lua_id, const char* text, int msg_type) {
	const CkManagedObject* managed = gObjectRegistry.get_managed(lua_id);
	if (!managed || !managed->ptr) return;

	fallout::Object* obj = managed->ptr;
	if (obj->elevation != fallout::gElevation) return;

	int color = fallout::_colorTable[32747], background_color = fallout::_colorTable[0], font = 101;

	switch (msg_type) {
		case 1: // (FLOATING_MESSAGE_TYPE_WHITE)
			color = fallout::_colorTable[32767];
			break;
		case 2: // (FLOATING_MESSAGE_TYPE_RED)
			color = fallout::_colorTable[31744];
			break;
		case 3: // (FLOATING_MESSAGE_TYPE_GREEN)
			color = fallout::_colorTable[992];
			break;
		case 4: // (FLOATING_MESSAGE_TYPE_BLUE)
			color = fallout::_colorTable[31];
			break;
		default: // YELLOW
			color = fallout::_colorTable[32747];
			break;
	}

	fallout::Rect rect;
	std::string converted = utf8_to_cp1251(text);

	if (fallout::textObjectAdd(obj, const_cast<char*>(converted.c_str()), font, color, background_color, &rect) != -1) {
		fallout::tileWindowRefreshRect(&rect, obj->elevation);
	}
}

int ck_critter_register(int pid, int tile) {
	return ck::register_critter(pid, tile);
}

