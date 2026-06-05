#include <iostream>

#include "map/ck_map.h"
#include "ck_scripting.h"
#include "ck_rendering.h"
#include "ck_debug_overlay/ck_debug_overlay.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}
extern lua_State* gLuaState;

void ck_scripting_on_map_enter() {
	ck_rendering_clear();
	if (ck_debug_overlay_enabled()) ck_debug_overlay_toggle();

	ck_call_hook("ckOnMapEnter");
}

void ck_map_add_scenery(int fid, int tile) {
	ck_rendering_add_scenery(fid, tile);
}

void ck_map_add_tile(int fid, int tile) {
	ck_rendering_add_tile(fid, tile);
}

