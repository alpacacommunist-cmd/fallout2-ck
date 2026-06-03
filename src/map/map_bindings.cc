// src/game_time/map_bindings.cc
#include "map/map_bindings.h"
#include "map.h" // ce map

extern "C" {
#include "../../src/vendor/luajit/src/lauxlib.h"
}

namespace {

	int l_ck_get_map_id(lua_State* L) {
		lua_pushinteger(L, fallout::mapGetCurrentMap());
		return 1;
	}

    const luaL_Reg map_lib[] = {
		{ "get_id", l_ck_get_map_id },
        { nullptr,          nullptr }
    };

}

int luaopen_ck_map(lua_State* L) {
    luaL_newlib(L, map_lib);
    return 1;
}
