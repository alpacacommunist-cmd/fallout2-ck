#include <iostream>

#include "map/ck_map.h"
#include "ck_rendering.h"
#include "ck_debug_overlay/ck_debug_overlay.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}
extern lua_State* gLuaState;

void ck_scripting_on_map_enter() {
    if (gLuaState == nullptr) return;

	ck_rendering_clear();
	if (ck_debug_overlay_enabled()) ck_debug_overlay_toggle();

    // search global lua table for function "ckOnMapEnter"
    lua_getglobal(gLuaState, "ckOnMapEnter");

    if (lua_isfunction(gLuaState, -1)) {
        int status = lua_pcall(gLuaState, 0, 0, 0);

        if (status != LUA_OK) {
            std::cerr << "[CK] Hook Error " << "(onMapEnter): " << lua_tostring(gLuaState, -1) << std::endl;
            lua_pop(gLuaState, 1);
        }
    } else {
        lua_pop(gLuaState, 1);
    }
}

