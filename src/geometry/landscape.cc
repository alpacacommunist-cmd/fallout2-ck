#include "object/ck_object.h"
#include "geometry/geometry.h"
#include "geometry/landscape.h"

#include "proto_types.h"

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

extern "C" {
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

		auto is_exit_grid = [](int obj_pid) {
			return obj_pid >= FIRST_EXIT_GRID_PID && obj_pid <= LAST_EXIT_GRID_PID;
		};

		ck_landscape_destroy_in_rect_match(rect, is_exit_grid);
	}
}
