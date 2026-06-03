// src/rendering/rendering_bindings.cc
#include "rendering/rendering_bindings.h"
#include "ck_rendering.h"

extern "C" {
#include "../../src/vendor/luajit/src/lauxlib.h"
}

namespace {

	static int l_draw_scenery(lua_State* L) {
		int fid = luaL_checkinteger(L, 1);
		int x = luaL_checkinteger(L, 2);
		int y = luaL_checkinteger(L, 3);

		ck_rendering_draw_scenery(fid, x, y);
		return 0;
	}

	static int l_add_scenery(lua_State* L) {
		int fid = luaL_checkinteger(L, 1);
		int tile = luaL_checkinteger(L, 2);

		ck_rendering_add_scenery(fid, tile);
		return 0;
	}

	static int l_add_tile(lua_State* L) {
		int fid = luaL_checkinteger(L, 1);
		int tile = luaL_checkinteger(L, 2);

		ck_rendering_add_tile(fid, tile);
		return 0;
	}

	static int l_set_camera_borders(lua_State* L) {
		int left = luaL_checkinteger(L, 1);
		int right = luaL_checkinteger(L, 2);
		int top = luaL_checkinteger(L, 3);
		int bottom = luaL_checkinteger(L, 4);

		ck_rendering_set_camera_borders(left, right, top, bottom);
		return 0;
	}

	static int l_clear_rendering(lua_State* L) {
		ck_rendering_clear();
		return 0;
	}

	static const luaL_Reg rendering_lib[] = {
		{ "draw_scenery", l_draw_scenery },
		{ "add_scenery", l_add_scenery },
		{ "add_tile", l_add_tile },
		{ "clear", l_clear_rendering },
		{ "set_camera_borders", l_set_camera_borders },
		{ nullptr, nullptr }
	};

}

int luaopen_ck_rendering(lua_State* L) {
    luaL_newlib(L, rendering_lib);
    return 1;
}
