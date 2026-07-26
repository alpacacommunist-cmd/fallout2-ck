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

	int find_at_tile(int tile, CkObjectFFI* buffer, int max_count) {
		int count = 0, current_elev = fallout::gElevation;

		fallout::Object* obj = fallout::objectFindFirstAtLocation(current_elev, tile);
		while (obj != nullptr && count < max_count) {
			buffer[count].c_ptr     = static_cast<void*>(obj);
			buffer[count].id        = obj->id;
			buffer[count].pid       = obj->pid;
			buffer[count].sid       = obj->sid;
			buffer[count].tile      = obj->tile;
			buffer[count].elevation = obj->elevation;
			buffer[count].flags     = obj->flags;
			buffer[count].rotation  = obj->rotation;

			buffer[count].lua_id    = ck::registry::find_by_ptr(obj);

			count++;
			log.info("found obj id: {}", obj->id);
			obj = fallout::objectFindNextAtLocation();
		}

		return count;
	}

	int find_by_pid(int pid, CkObjectFFI* buffer, int max_count) {
        int count = 0;
        int current_elev = fallout::gElevation;

        fallout::Object* obj = fallout::objectFindFirstAtElevation(current_elev);

        while (obj != nullptr && count < max_count) {
            if (obj->pid == pid) {
                buffer[count].c_ptr     = static_cast<void*>(obj);
                buffer[count].id        = obj->id;
                buffer[count].pid       = obj->pid;
                buffer[count].sid       = obj->sid;
                buffer[count].tile      = obj->tile;
                buffer[count].elevation = obj->elevation;
                buffer[count].flags     = obj->flags;
                buffer[count].rotation  = obj->rotation;

                buffer[count].lua_id    = ck::registry::find_by_ptr(obj);

				log.info("found obj id: {}", obj->id);
                count++;
            }

            obj = fallout::objectFindNextAtElevation();
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
