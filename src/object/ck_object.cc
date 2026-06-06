#include "ck_scripting.h"
#include "object/ck_object.h"
#include "tile.h"

void ck_object_remove_blocker_at(int tile) {
	fallout::Object* blocker = fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
	bool blocking = (blocker != nullptr && (FID_TYPE(blocker->fid) != fallout::OBJ_TYPE_CRITTER));

	if (blocking) fallout::objectDestroy(blocker, nullptr);
}

void ck_object_create_blocker_at(int tile) {
	fallout::Object* blocker = nullptr;
	fallout::objectCreateWithFidPid(&blocker, 0x2000015, 0x2000158);
	fallout::objectSetLocation(blocker, tile, fallout::gElevation, nullptr);
}

