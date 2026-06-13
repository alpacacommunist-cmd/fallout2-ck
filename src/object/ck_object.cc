#include "ck_scripting.h"
#include "object/ck_object.h"
#include "tile.h"

const int BLOCKER_PID=0x2000158; // dummy collision object
const int BLOCKER_FID=0x02000015;

const int SCRIPT_ID           = 13;
const int SCRIPT_TYPE_CRITTER = 4;
const int OBJECT_LUA_MANAGED  = 0x08000000;

static fallout::Object* ck_object_blocker_at(int tile) {
	return fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
}

bool ck_object_blocking(int tile) {
	fallout::Object* blocker = ck_object_blocker_at(tile);

	return blocker != nullptr; // something blocks tile
}

void ck_object_create_at(int fid, int tile) {
	if (ck_object_blocking(tile)) return;

	fallout::Object* object = nullptr;
	fallout::objectCreateWithFidPid(&object, fid, BLOCKER_PID);
	fallout::objectSetLocation(object, tile, fallout::gElevation, nullptr);

	if (object) object->flags |= fallout::OBJECT_NO_SAVE;
}
void ck_object_create_persistent_at(int fid, int tile);

void ck_object_remove_blocker_at(int tile) {
	fallout::Object* blocker = ck_object_blocker_at(tile);

	if (blocker != nullptr && (FID_TYPE(blocker->fid) == fallout::OBJ_TYPE_SCENERY))
		fallout::objectDestroy(blocker, nullptr);
}

void ck_object_create_blocker_at(int tile) {
	ck_object_create_at(BLOCKER_FID, tile);
}

void ck_object_critter_create(int pid, int tile, int lua_script_id) {
	if (ck_object_blocking(tile)) return;

    fallout::Object* critter = nullptr;

    if (fallout::objectCreateWithPid(&critter, pid) == 0) {
        fallout::objectSetLocation(critter, tile, fallout::gElevation, nullptr);
		critter->sid = (SCRIPT_TYPE_CRITTER << 24) | SCRIPT_ID;

		critter->flags |= fallout::OBJECT_NO_SAVE;
		critter->flags |= OBJECT_LUA_MANAGED;
    }
}
