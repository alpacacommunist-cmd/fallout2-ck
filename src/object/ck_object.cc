#include "ck_ids.h"
#include "object/ck_object.h"
#include "object/ck_object_registry.h"
#include "object/ck_critter.h"

#include "tile.h"
#include "proto.h"

#include <ostream>
#include <iostream>

const int BLOCKER_PID=0x2000158;  // dummy collision object
const int BLOCKER_FID=0x02000015;

static fallout::Object* ck_object_blocker_at(int tile) {
	return fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
}

void ck_object_remove_managed(fallout::Object* obj) {
    if (obj == nullptr) return;
    bool found_in_registry = gObjectRegistry.remove_by_ptr(obj);
    if (found_in_registry) { std::cout << "[CK Debug] Object removed from registry before destruction." << std::endl; }

    fallout::objectDestroy(obj, nullptr);
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

int ck_object_register_object(int pid, int tile) {
	fallout::Object* object = ck_object_create_at(pid, tile);

	if (object != nullptr) return gObjectRegistry.add(object);

	return -1;
}

void ck_object_remove_at(int tile) {
	std::vector<fallout::Object*> to_delete;
	fallout::Object* object = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);

	if (object == nullptr) return;

	while (object != nullptr) {
		fallout::Object* next_object = fallout::objectFindNextAtLocation();
		to_delete.push_back(object); object = next_object;
	}

    for (fallout::Object* object : to_delete) { ck_object_remove_managed(object); }
}

void ck_object_remove_blocker_at(int tile) {
	fallout::Object* blocker = ck_object_blocker_at(tile);

	if (blocker != nullptr && (FID_TYPE(blocker->fid) == fallout::OBJ_TYPE_SCENERY)) {
		ck_object_remove_managed(blocker);
	}
}

void ck_object_create_blocker_at(int tile) {
	ck_object_create_at(BLOCKER_FID, tile);
}

// ffi
int player_stat(int stat) {
	return ck::critter_stat(fallout::gDude, stat);
}

int player_pc_stat(int stat) {
	return ck::critter_pc_stat(stat);
}

int ck_object_get_sid(int lua_id) {
	const CkManagedObject* managed = gObjectRegistry.get_managed(lua_id);

	if (!managed || !managed->ptr) return -1;
	if (!ck::is_ck_sid(ck::clean_sid(managed->ptr->sid))) return -1;

	return ck::clean_sid(managed->ptr->sid);
}

void* ck_object_get_ptr(int lua_id) {
	const CkManagedObject* managed = gObjectRegistry.get_managed(lua_id);
	if (!managed) return nullptr;
	return static_cast<void*>(managed->ptr);
}

int ck_object_get_tile(int lua_id) {
	const CkManagedObject* managed = gObjectRegistry.get_managed(lua_id);
	if (!managed || !managed->ptr) return -1;

	return managed->ptr->tile;
}

