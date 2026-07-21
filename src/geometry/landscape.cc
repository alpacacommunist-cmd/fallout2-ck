#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "geometry/geometry.h"
#include "geometry/landscape.h"

#include "proto_types.h"
#include "object.h"

static void ck_landscape_destroy_in_rect_match(const HexRect& rect, std::function<bool(int pid)> should_destroy) {
    std::vector<fallout::Object*> to_delete;

    rect.for_each_tile([&to_delete, &should_destroy](int tile) {
        fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);
        while (obj != nullptr) {
            fallout::Object* next_obj = fallout::objectFindNextAtLocation();

            if (should_destroy(obj->pid)) { to_delete.push_back(obj); }
            obj = next_obj;
        }
    });

    for (fallout::Object* obj : to_delete) { fallout::objectDestroy(obj, nullptr); }
}

static void ck_landscape_hide_in_rect_match(const HexRect& rect, std::function<bool(int pid)> should_hide) {
    rect.for_each_tile([&should_hide](int tile) {
        fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);
        while (obj != nullptr) {
            fallout::Object* next_obj = fallout::objectFindNextAtLocation();

            if (should_hide(obj->pid) && FID_TYPE(obj->pid) != fallout::OBJ_TYPE_CRITTER) {
                obj->flags |= fallout::OBJECT_HIDDEN;
            }
            obj = next_obj;
        }
    });
}

void ck_landscape_toggle_visibility_in_rect(const HexRect& rect, bool visible) {
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

// FFI
int ck_proto_first_exit_grid_pid() { return FIRST_EXIT_GRID_PID; }
int ck_proto_last_exit_grid_pid()  { return LAST_EXIT_GRID_PID; }

void ck_landscape_destroy_pid_in_rect(int left, int right, int top, int bottom, int pid) {
	std::vector<int> tiles = { left, right, top, bottom };
	HexRect rect = geometry_build_rect_from_points(tiles);

	if (!rect.is_valid()) return;

	ck_landscape_destroy_in_rect_match(rect, [pid](int obj_pid) { return obj_pid == pid; });
}

void ck_landscape_destroy_exit_grid_in_rect(int left, int right, int top, int bottom) {
    std::vector<int> tiles = { left, right, top, bottom };
    HexRect rect = geometry_build_rect_from_points(tiles);
    if (!rect.is_valid()) return;

    std::string current_mod = ck_get_current_mod_id();

    rect.for_each_tile([&current_mod](int tile) {
        fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);
        while (obj != nullptr) {
            fallout::Object* next_obj = fallout::objectFindNextAtLocation();

            if (obj->pid >= FIRST_EXIT_GRID_PID && obj->pid <= LAST_EXIT_GRID_PID) {
				ck::registry::deleted::add(obj, current_mod);
            }
            obj = next_obj;
        }
    });
}

void ck_landscape_create_exit_grid_in_rect(int t1, int t2, int t3, int t4, int pid, CKExitGridData data) {
	std::vector<int> tiles = { t1, t2, t3, t4 };
	HexRect rect = geometry_build_rect_from_points(tiles);

	if (!rect.is_valid()) return;

	rect.for_each_tile([pid, &data](int tile) {
		const LuaMeta& meta  = { ck_get_current_mod_id(), {}, {}, {} };
		int lua_id = ck_object_register_object(pid, tile, meta);

		fallout::Object* obj = ck::registry::created::get_object(lua_id);

		if (obj != nullptr) {
			obj->data.misc.map       = data.target_map;
			obj->data.misc.tile      = data.target_tile;
			obj->data.misc.elevation = data.target_elevation;
			obj->data.misc.rotation  = data.target_rotation;
		}
	});
}

void ck_landscape_create_exit_grid_at_tile(int tile, int pid, const CKExitGridData* data) {
    if (data == nullptr) return;

    const LuaMeta& meta = { ck_get_current_mod_id(), {}, {}, {} };
    int lua_id = ck_object_register_object(pid, tile, meta);

    fallout::Object* obj = ck::registry::created::get_object(lua_id);
    if (obj != nullptr) {
        obj->data.misc.map       = data->target_map;
        obj->data.misc.tile      = data->target_tile;
        obj->data.misc.elevation = data->target_elevation;
        obj->data.misc.rotation  = data->target_rotation;
    }
}
