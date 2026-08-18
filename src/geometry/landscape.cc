#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "geometry/geometry.h"
#include "geometry/landscape.h"

#include "proto_types.h"
#include "obj_types.h"

void ck_landscape_toggle_visibility_in_rect(const HexRect& rect, bool visible) {
    rect.for_each_tile([visible](int tile) {
        fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);

        while (obj != nullptr) {
            fallout::Object* nextObj = fallout::objectFindNextAtLocation();

            if (fallout::objectTypeFromFid(obj->fid) != fallout::OBJ_TYPE_CRITTER && obj != fallout::gEgg) {
                if (visible) {
                    obj->flags &= ~fallout::OBJECT_HIDDEN;

                    std::erase_if(ck::registry::g_deleted_objects, [obj](const auto& entry) {
                        return entry.ptr == obj;
                    });
                }
                else {
                    bool already_tracked = std::any_of(
                        ck::registry::g_deleted_objects.begin(),
                        ck::registry::g_deleted_objects.end(),
                        [obj](const auto& entry) { return entry.ptr == obj; }
                    );

                    if (!already_tracked) {
                        ck::registry::deleted::add(obj);
                    }
                }
            }

            obj = nextObj;
        }
	});
}

// FFI
int ck_proto_first_exit_grid_pid()  { return FIRST_EXIT_GRID_PID; }
int ck_proto_last_exit_grid_pid()   { return LAST_EXIT_GRID_PID; }
int ck_proto_exit_grid_marker_pid() { return PROTO_ID_EXIT_GRID_MAP_MARKER; }

void ck_landscape_destroy_exit_grid_in_rect(int left, int right, int top, int bottom) {
    std::vector<int> tiles = { left, right, top, bottom };
    HexRect rect = geometry_build_rect_from_points(tiles);
    if (!rect.is_valid()) return;

    rect.for_each_tile([](int tile) {
        fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);
        while (obj != nullptr) {
            fallout::Object* next_obj = fallout::objectFindNextAtLocation();

            if (obj->pid >= FIRST_EXIT_GRID_PID && obj->pid <= LAST_EXIT_GRID_PID) {
				ck::registry::deleted::add(obj);
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
		int lua_id = ck_object_register(pid, tile, meta);

		fallout::Object* obj = ck::registry::created::get_object(lua_id);

		if (obj != nullptr) {
			obj->data.misc.map       = fallout::Map(data.target_map);
			obj->data.misc.tile      = data.target_tile;
			obj->data.misc.elevation = data.target_elevation;
			obj->data.misc.rotation  = static_cast<fallout::Rotation>(data.target_rotation);
		}
	});
}

void ck_landscape_create_exit_grid_at_tile(int tile, int pid, const CKExitGridData* data) {
    if (data == nullptr) return;

    const LuaMeta& meta = { ck_get_current_mod_id(), {}, {}, {} };
    int lua_id = ck_object_register(pid, tile, meta);

    fallout::Object* obj = ck::registry::created::get_object(lua_id);
    if (obj != nullptr) {
        obj->data.misc.map       = fallout::Map(data->target_map);
        obj->data.misc.tile      = data->target_tile;
        obj->data.misc.elevation = data->target_elevation;
        obj->data.misc.rotation  = static_cast<fallout::Rotation>(data->target_rotation);
    }
}
