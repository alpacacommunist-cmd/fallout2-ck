#include "ck_scripting.h"
#include "object/ck_object.h"
#include "tile.h"

const int BLOCKER_PID=0x2000158; // dummy collision object

void ck_object_remove_blocker_at(int tile) {
	fallout::Object* blocker = fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
	bool blocking = (blocker != nullptr && (FID_TYPE(blocker->fid) != fallout::OBJ_TYPE_CRITTER));

	if (blocking) fallout::objectDestroy(blocker, nullptr);
}

void ck_object_create_at(int fid, int tile) {
	fallout::Object* object = nullptr;
	fallout::objectCreateWithFidPid(&object, fid, BLOCKER_PID);
	fallout::objectSetLocation(object, tile, fallout::gElevation, nullptr);
}

void ck_object_create_blocker_at(int tile) {
	ck_object_create_at(0x2000015, tile);
}
