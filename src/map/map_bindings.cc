// src/game_time/map_bindings.cc
#include "map/map_bindings.h"
#include "map.h" // ce map
#include "map/ck_map.h" // ck map

extern "C" {
#include "../../src/vendor/luajit/src/lauxlib.h"
}

namespace {

	int l_get_map_id(lua_State* L) {
		lua_pushinteger(L, fallout::mapGetCurrentMap());
		return 1;
	}

	int l_add_scenery(lua_State* L) {
		int fid = luaL_checkinteger(L, 1);
		int tile = luaL_checkinteger(L, 2);

		ck_map_add_scenery(fid, tile);
		return 0;
	}

	int l_add_tile(lua_State* L) {
		int fid = luaL_checkinteger(L, 1);
		int tile = luaL_checkinteger(L, 2);

		ck_map_add_tile(fid, tile);
		return 0;
	}

    const luaL_Reg map_lib[] = {
		{ "get_id", l_get_map_id },
		{ "add_scenery", l_add_scenery },
		{ "add_tile", l_add_tile },
        { nullptr,          nullptr }
    };

}

int luaopen_ck_map(lua_State* L) {
    luaL_newlib(L, map_lib);
    return 1;
}
