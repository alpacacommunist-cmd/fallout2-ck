#include "ck_ids.h"
#include "object/ck_object.h"
#include "object/ck_object_registry.h"
#include "tile.h"

const int BLOCKER_PID=0x2000158;  // dummy collision object
const int BLOCKER_FID=0x02000015;

const int SCRIPT_ID           = 13;
const int SCRIPT_TYPE_CRITTER = 4;

static fallout::Object* ck_object_blocker_at(int tile) {
	return fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
}

bool ck_object_blocking(int tile) {
	fallout::Object* blocker = ck_object_blocker_at(tile);

	return blocker != nullptr; // something blocks tile
}

fallout::Object* ck_object_create_at(int fid, int tile) {
	if (ck_object_blocking(tile)) return nullptr;

	fallout::Object* object = nullptr;

	if (fallout::objectCreateWithFidPid(&object, fid, BLOCKER_PID) == 0) {
		fallout::objectSetLocation(object, tile, fallout::gElevation, nullptr);
		object->flags |= fallout::OBJECT_NO_SAVE;

		return object;
	}

	return nullptr;
}
void ck_object_create_persistent_at(int fid, int tile);

fallout::Object* ck_object_create_critter(int pid, int tile) {
	if (ck_object_blocking(tile)) return nullptr;

	fallout::Object* critter = nullptr;

	if (fallout::objectCreateWithPid(&critter, pid) == 0) {
		fallout::objectSetLocation(critter, tile, fallout::gElevation, nullptr);
		critter->sid = (SCRIPT_TYPE_CRITTER << 24) | SCRIPT_ID;

		critter->flags |= fallout::OBJECT_NO_SAVE;

		return critter;
	}

	return nullptr;
}

int ck_object_register_object(int pid, int tile) {
	fallout::Object* object = ck_object_create_at(pid, tile);

	if (object != nullptr) return gObjectRegistry.add(object);

	return -1;
}

int ck_object_register_critter(int pid, int tile) {
	fallout::Object* critter = ck_object_create_critter(pid, tile);

	int lua_id = -1;

	if (critter != nullptr) {
		lua_id = gObjectRegistry.add(critter);

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

