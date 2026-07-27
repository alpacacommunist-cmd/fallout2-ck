#include "ck_ids.h"
#include "object/ck_object.h"
#include "object/ck_item.h"
#include "ck_registry/ck_registry.h"

#include "object.h"

#include "ck_log.h"
static const Logger log("CK Object");

static fallout::Object* ck_object_blocker_at(int tile) {
	return fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
}

namespace ck::object {

namespace {
	void build_ffi_struct(CkObjectFFI& destination, fallout::Object* source) {
		if (!source) return;

		destination.c_ptr     = static_cast<void*>(source);
		destination.id        = source->id;
		destination.pid       = source->pid;
		destination.sid       = source->sid;
		destination.tile      = source->tile;
		destination.elevation = source->elevation;
		destination.flags     = source->flags;
		destination.rotation  = source->rotation;

		destination.lua_id    = ck::registry::find_by_ptr(source);
	}
}

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

	int find_at_tile(int tile, CkObjectFFI* buffer, int max_count) {
		int count = 0, elevation = fallout::gElevation;

        fallout::Object* object = fallout::objectFindFirstAtLocation(elevation, tile);
        while (object != nullptr && count < max_count) {
            build_ffi_struct(buffer[count], object);

            count++;
            object = fallout::objectFindNextAtLocation();
        }

        return count;
	}

	int find_by_pid(int pid, CkObjectFFI* buffer, int max_count) {
		int count = 0, elevation = fallout::gElevation;

		fallout::Object* object = fallout::objectFindFirstAtElevation(elevation);
		while (object != nullptr && count < max_count) {
			if (object->pid == pid) {
				build_ffi_struct(buffer[count], object);
				count++;
			}

			object = fallout::objectFindNextAtElevation();
		}

		return count;
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
	if (!ck::ids::is_ck_sid(ck::ids::clean_sid(object->ptr->sid))) return -1;

	return ck::ids::clean_sid(object->ptr->sid);
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

// fallout id
int ck_object_get_id(int lua_id) {
	fallout::Object* object = ck::registry::get_object(lua_id);
	if (!object) return -1;

	return object->id;
}

int ck_object_find_at_tile(int tile, CkObjectFFI* buffer, int max_count) {
	return ck::object::find_at_tile(tile, buffer, max_count);
}

int ck_object_find_by_pid(int pid, CkObjectFFI* buffer, int max_count) {
    return ck::object::find_by_pid(pid, buffer, max_count);
}
