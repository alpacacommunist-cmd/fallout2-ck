#include "ck_utils.h"
#include "object/ck_object.h"
#include "object/ck_item.h"
#include "ck_registry/ck_registry.h"

#include "object.h"

#include "ck_log.h"
static const Logger log("CK Object");

static fallout::Object* ck_object_blocker_at(int tile, int elevation) {
	return fallout::_obj_blocking_at(nullptr, tile, elevation);
}

namespace ck::critter::proto {
    extern bool has_custom_prototype(int pid);
}

namespace ck::common {
    const char* system_mod_id();
    const char* current_mod_id();
}

namespace ck::object {
	void to_ffi(CkObjectFFI& destination, fallout::Object* source) {
		if (!source) return;

		destination.c_ptr     = static_cast<void*>(source);
		destination.id        = source->id;
		destination.pid       = source->pid;
		destination.sid       = source->sid;
		destination.tile      = source->tile;
		destination.elevation = source->elevation;
		destination.flags     = source->flags;
		destination.rotation  = source->rotation;

		destination.name      = fallout::objectGetName(source);
		destination.lua_id    = ck::registry::find_by_ptr(source);
        destination.mod_id    = ck::common::system_mod_id();

        const char* current_mod_id = ck::common::current_mod_id();
        if (current_mod_id != nullptr) destination.mod_id = current_mod_id;

        if (destination.lua_id != -1) {
            const LuaMeta* object_meta = registry::get_meta(destination.lua_id);

            if (object_meta) destination.mod_id = object_meta->mod_id.c_str();
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
            to_ffi(buffer[count], object);

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
				to_ffi(buffer[count], object);
				count++;
			}

			object = fallout::objectFindNextAtElevation();
		}

		return count;
	}

    int type(int pid) {
        return static_cast<int>(fallout::objectTypeFromPid(pid));
    }
}

bool ck_object_blocking(int tile, int elevation) {
	fallout::Object* blocker = ck_object_blocker_at(tile, elevation);

	return blocker != nullptr; // something blocks tile
}

fallout::Object* ck_object_create(int pid, int tile, int elevation, bool search_free_tile) {
	if (ck_object_blocking(tile, elevation) && !search_free_tile) return nullptr;

	fallout::Object* object = nullptr;
	if (fallout::objectCreateWithPid(&object, pid) == 0) {
		object->flags |= fallout::OBJECT_NO_SAVE;
		object->flags |= fallout::OBJECT_CK;

        if (ck::object::type(pid) == 1 && ck::critter::proto::has_custom_prototype(pid) > 0) {
            object->flags |= fallout::OBJECT_CK_PROTO;
            log.debug("Object created with custom prototype flag. PID: {}", pid);
        }
	}

    if (ck_object_blocking(tile, elevation)) {
        const int radius = 3;

        for (int direction = 0; direction < 6; direction++) {
            for (int distance = 1; distance <= radius; distance++) {
                int new_tile = fallout::tileGetTileInDirection(tile, static_cast<fallout::Rotation>(direction), distance);

                if (!hexGridTileIsValid(new_tile)) continue;
                if (ck_object_blocking(new_tile, elevation)) continue;

                tile = new_tile;
            }
        }
    }

    if (object) {
        fallout::objectSetLocation(object, tile, elevation, nullptr);
        return object;
    }

	return nullptr;
}

int ck_object_register(int pid, int tile, const LuaMeta& meta) {
	fallout::Object* object = ck_object_create(pid, tile);

	if (object != nullptr) return ck::registry::created::add(object, meta);

	return -1;
}

// ffi

void ck_object_remove_at(int tile) {
	ck::object::remove_at(tile);
}

void* ck_object_get_ptr(int lua_id) {
	fallout::Object* object = ck::registry::get_object(lua_id);

	if (!object) return nullptr;
	return object;
}

// fallout id
int ck_object_get_id(void* ptr) {
	if (!ptr) return -1; fallout::Object* object = static_cast<fallout::Object*>(ptr);
	return object->id;
}

int ck_object_get_pid(fallout::Object* object) {
    CK_ENSURE_VALID_OBJECT(object);
    return object->pid;
}

int ck_object_get_tile(void* ptr) {
	if (!ptr) return -1; fallout::Object* object = static_cast<fallout::Object*>(ptr);
	return object->tile;
}

int ck_object_get_sid(void* ptr) {
	if (!ptr) return -1; fallout::Object* object = static_cast<fallout::Object*>(ptr);
	return object->tile;
}

char* ck_object_get_name(void* ptr) {
	if (!ptr) return nullptr; fallout::Object* object = static_cast<fallout::Object*>(ptr);
	return fallout::objectGetName(object);
}

int ck_object_get_type(void* ptr) {
	if (!ptr) return -1; fallout::Object* object = static_cast<fallout::Object*>(ptr);
	return ck::object::type(object->pid);
}

int ck_object_find_at_tile(int tile, CkObjectFFI* buffer, int max_count) {
	return ck::object::find_at_tile(tile, buffer, max_count);
}

int ck_object_find_by_pid(int pid, CkObjectFFI* buffer, int max_count) {
    return ck::object::find_by_pid(pid, buffer, max_count);
}

