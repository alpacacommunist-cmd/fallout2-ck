#include "ck_ids.h"
#include "object/ck_object.h"
#include "object/ck_item.h"

#include "tile.h"
#include "proto.h"

#include "ck_log.h"
static const Logger log("CK Object");

static fallout::Object* ck_object_blocker_at(int tile) {
	return fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
}

namespace ck::object {
	void remove_at(int tile) {
		std::vector<fallout::Object*> to_delete;
		fallout::Object* object = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);

		if (object == nullptr) return;

		while (object != nullptr) {
			fallout::Object* next_object = fallout::objectFindNextAtLocation();
			to_delete.push_back(object); object = next_object;
		}

		for (fallout::Object* object : to_delete) ck::registry::deleted::add(object);
	}
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

int ck_object_register_object_by_fid(int fid, int tile, const LuaMeta& meta) {
	fallout::Object* object = ck_object_create_at(fid, tile);

	if (object != nullptr) return ck::registry::created::add(object, meta);

	return -1;
}

int ck_object_register_object(int pid, int tile, const LuaMeta& meta) {
	fallout::Object* object = ck_object_create(pid, tile);

	if (object != nullptr) return ck::registry::created::add(object, meta);

	return -1;
}

// ffi

void ck_object_remove_at(int tile) {
	ck::object::remove_at(tile);
}

int ck_object_get_sid(int lua_id) {
	const CkCreatedObject* object = ck::registry::created::get(lua_id);

	if (!object || !object->ptr) return -1;
	if (!ck::is_ck_sid(ck::clean_sid(object->ptr->sid))) return -1;

	return ck::clean_sid(object->ptr->sid);
}

void* ck_object_get_ptr(int lua_id) {
	const CkCreatedObject* object = ck::registry::created::get(lua_id);
	if (!object) return nullptr;
	return static_cast<void*>(object->ptr);
}

int ck_object_get_tile(int lua_id) {
	const CkCreatedObject* object = ck::registry::created::get(lua_id);
	if (!object || !object->ptr) return -1;

	return object->ptr->tile;
}

