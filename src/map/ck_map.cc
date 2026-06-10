#include <iostream>
#include "ck_scripting.h"
#include "ck_rendering.h"
#include "map/ck_map.h"
#include "object/ck_object.h"
#include "ck_debug_overlay/ck_debug_overlay.h"

#include "tile.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}
extern lua_State* gLuaState;

static CkCameraBorders gCameraBorders;

void ck_scripting_on_map_enter() {
	ck_rendering_clear();
	ck_map_clear_camera_borders();

	if (ck_debug_overlay_enabled()) ck_debug_overlay_toggle();

	ck_call_hook("ckOnMapEnter");
}

void ck_map_add_scenery(int fid, int tile) {
	ck_rendering_add_scenery(fid, tile);
}

void ck_map_add_scenery(const std::string& key, int tile) {
    ck_rendering_add_custom_scenery(key, tile);
}

void ck_map_add_tile(int fid, int tile) {
	ck_rendering_add_tile(fid, tile);
}

void ck_map_add_tile(const std::string& key, int tile) {
    ck_rendering_add_custom_tile(key, tile);
}

void ck_map_remove_blocker(int tile) {
	ck_object_remove_blocker_at(tile);
}

void ck_map_create_blocker(int tile) {
	ck_object_create_blocker_at(tile);
}

void ck_map_create_object(int artId, int tile) {
	int fid = (fallout::OBJ_TYPE_SCENERY << 24) | (artId & 0x0000FFFF);
	ck_object_create_at(fid, tile);
}

void ck_map_create_object_fid(int fid, int tile) {
	ck_object_create_at(fid, tile);
}

bool ck_map_is_camera_position_allowed(int tile) {
    if (!gCameraBorders.enabled) { return false; }

    int gridWidth = fallout::tileGetHexGridWidth();
    int tileX = gridWidth - 1 - tile % gridWidth;
    int tileY = tile / gridWidth;

    bool allowed = tileX >= gCameraBorders.left && tileX <= gCameraBorders.right &&
        tileY >= gCameraBorders.top && tileY <= gCameraBorders.bottom;

    // fallout::debugPrint("[CK] Camera check " "tile=(%d,%d) " "bounds=(%d..%d,%d..%d) " "allowed=%d\n",
    //     tileX, tileY, gCameraBorders.left, gCameraBorders.right, gCameraBorders.top, gCameraBorders.bottom, allowed);

	return allowed;
}

bool ck_map_has_camera_borders() { return gCameraBorders.enabled; }

void ck_map_set_camera_borders(int left, int right, int top, int bottom) {
    gCameraBorders.enabled = true;

    gCameraBorders.left = left;
    gCameraBorders.right = right;
    gCameraBorders.top = top;
    gCameraBorders.bottom = bottom;
}

void ck_map_clear_camera_borders() { gCameraBorders = {}; }
const CkCameraBorders& ck_map_get_camera_borders() { return gCameraBorders; }
