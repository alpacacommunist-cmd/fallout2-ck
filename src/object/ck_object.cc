#include "ck_ids.h"
#include "object/ck_object.h"

#include "tile.h"
#include "proto.h"

#include <ostream>
#include <iostream>

const int BLOCKER_PID=0x2000158;  // dummy collision object
const int BLOCKER_FID=0x02000015;

static fallout::Object* ck_object_blocker_at(int tile) {
	return fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
}

bool ck_object_blocking(int tile) {
	fallout::Object* blocker = ck_object_blocker_at(tile);

	return blocker != nullptr; // something blocks tile
}

fallout::Object* ck_object_create(int pid, int tile) {
	if (ck_object_blocking(tile)) return nullptr;

	fallout::Object* object = nullptr;

	if (fallout::objectCreateWithPid(&object, pid) == 0) {
		fallout::objectSetLocation(object, tile, fallout::gElevation, nullptr);
		object->flags |= fallout::OBJECT_NO_SAVE;

		return object;
	}

	return nullptr;
}

fallout::Object* ck_object_create_at(int fid, int tile) {
	fallout::Object* object = ck_object_create(BLOCKER_PID, tile);

	if (object == nullptr) return nullptr;
	object->fid = fid;

	return object;
}

fallout::Object* ck_object_create_critter(int pid, int tile) {
	if (ck_object_blocking(tile)) return nullptr;

	fallout::Object* critter = nullptr;

	if (fallout::objectCreateWithPid(&critter, pid) == 0) {
		fallout::objectSetLocation(critter, tile, fallout::gElevation, nullptr);

		critter->flags |= fallout::OBJECT_NO_SAVE;
		critter->sid    = ck::make_full_sid(fallout::SCRIPT_TYPE_CRITTER, fallout::gMapSid);

		return critter;
	}

	return nullptr;
}

int ck_object_register_object(int pid, int tile) {
	fallout::Object* object = ck_object_create_at(pid, tile);

	if (object != nullptr) return gObjectRegistry.add(object);

	return -1;
}

int ck_object_register_critter(int pid, int tile, const LuaCritterMeta& meta) {
	fallout::Object* critter = ck_object_create_critter(pid, tile);

	int lua_id = -1;

	LuaCritterMeta full_meta = meta;
	full_meta.proto_sid      = critter->sid;

	if (critter != nullptr) {
		lua_id = gObjectRegistry.add(critter, full_meta);

		int custom_sid = ck::make_sid(lua_id);
		critter->sid   = ck::make_full_sid(fallout::SCRIPT_TYPE_CRITTER, custom_sid);
	}

	return lua_id;
}

void ck_object_remove_blocker_at(int tile) {
	fallout::Object* blocker = ck_object_blocker_at(tile);

	if (blocker != nullptr && (FID_TYPE(blocker->fid) == fallout::OBJ_TYPE_SCENERY))
		fallout::objectDestroy(blocker, nullptr);
}

void ck_object_create_blocker_at(int tile) {
	ck_object_create_at(BLOCKER_FID, tile);
}

