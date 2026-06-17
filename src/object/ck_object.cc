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

void ck_object_remove_all_by_pid(int pid) {
	for (int tile = 0; tile < 40000; ++tile) {
		fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);

		while (obj != nullptr) {
			fallout::Object* nextObj = fallout::objectFindNextAtLocation();

			if (obj->pid == pid) {
				fallout::objectDestroy(obj, nullptr);
				std::cout << "[CK] Removed object PID " << pid << " at tile " << tile << std::endl;
			}

			obj = nextObj;
		}
	}
}

void ck_object_create_pid_at(int pid, int tile) {
    if (tile < 0 || tile >= 40000) return;

    fallout::Object* obj = nullptr;
    if (fallout::objectCreateWithPid(&obj, pid) == 0) {
        fallout::objectSetLocation(obj, tile, fallout::gElevation, nullptr);

        // if (pid == 83886100 || pid == 83886098 || pid == 33554481) {
        //     obj->flags |= fallout::OBJECT_HIDDEN;
        // }

        obj->flags |= fallout::OBJECT_NO_SAVE;

        std::cout << "[CK] Spawned PID " << pid << " at tile " << tile << std::endl;
    }
}

void ck_object_toggle_visibility_in_rect(const HexRect& rect, bool visible) {
    rect.for_each_tile([visible](int tile) {
        fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);

        while (obj != nullptr) {
            fallout::Object* nextObj = fallout::objectFindNextAtLocation();

            if (FID_TYPE(obj->fid) != fallout::OBJ_TYPE_CRITTER) {
                if (visible) obj->flags &= ~fallout::OBJECT_HIDDEN;
                else obj->flags |= fallout::OBJECT_HIDDEN;
            }

            obj = nextObj;
        }
    });
}

extern "C" {
    void ck_map_remove_all_by_pid(int pid) { ck_object_remove_all_by_pid(pid); }
    void ck_map_create_pid_at(int pid, int tile) { ck_object_create_pid_at(pid, tile); }
}
