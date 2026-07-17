#ifndef CK_LANDSCAPE_H
#define CK_LANDSCAPE_H

#include "ck_api.h"

extern "C" const char* ck_get_current_mod_id();

namespace fallout {
	extern int gElevation;
	int tileGetRotationTo(int tile1, int tile2);
	int tileGetTileInDirection(int tile, int rotation, int distance);
}

struct CKExitGridData {
    int target_map;
    int target_tile;
    int target_elevation;
    int target_rotation;
};

static void ck_landscape_destroy_in_rect_match(const HexRect& rect, std::function<bool(int pid)> should_destroy);
void ck_landscape_toggle_visibility_in_rect(const HexRect& rect, bool visible);

CK_API int ck_proto_first_exit_grid_pid();
CK_API int ck_proto_last_exit_grid_pid();
CK_API void ck_landscape_create_exit_grid_in_rect(int t1, int t2, int t3, int t4, int pid, CKExitGridData data);
CK_API void ck_landscape_create_exit_grid_at_tile(int tile, int pid, const CKExitGridData* data);
CK_API void ck_landscape_destroy_pid_in_rect(int left, int right, int top, int bottom, int pid);
CK_API void ck_landscape_destroy_exit_grid_in_rect(int left, int right, int top, int bottom);

#endif
