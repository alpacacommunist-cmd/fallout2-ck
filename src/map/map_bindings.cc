// src/map/map_bindings.cc
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
		int tile = luaL_checkinteger(L, 2);

		if (lua_type(L, 1) == LUA_TSTRING) {
			const char* key = lua_tostring(L, 1);
			ck_map_add_scenery(std::string(key), tile);
		} else {
			int fid = luaL_checkinteger(L, 1);
			ck_map_add_scenery(fid, tile);
		}
		return 0;
	}

	int l_add_tile(lua_State* L) {
		int tile = luaL_checkinteger(L, 2);

		if (lua_type(L, 1) == LUA_TSTRING) {
			const char* key = lua_tostring(L, 1);
			ck_map_add_tile(std::string(key), tile);
		} else {
			int fid = luaL_checkinteger(L, 1);
			ck_map_add_tile(fid, tile);
		}
		return 0;
	}

	int l_set_camera_borders(lua_State* L) {
		int left = luaL_checkinteger(L, 1);
		int right = luaL_checkinteger(L, 2);
		int top = luaL_checkinteger(L, 3);
		int bottom = luaL_checkinteger(L, 4);

		ck_map_set_camera_borders(left, right, top, bottom);
		return 0;
	}

	int l_remove_blocker(lua_State* L) {
		int tile = luaL_checkinteger(L, 1);

		ck_map_remove_blocker(tile);
		return 0;
	}

	int l_create_blocker(lua_State* L) {
		int tile = luaL_checkinteger(L, 1);

		ck_map_create_blocker(tile);
		return 0;
	}

	int l_create_object(lua_State* L) {
		int fid = luaL_checkinteger(L, 1);
		int tile = luaL_checkinteger(L, 2);

		ck_map_create_object(fid, tile);
		return 0;
	}

	int l_create_object_fid(lua_State* L) {
		int fid = luaL_checkinteger(L, 1);
		int tile = luaL_checkinteger(L, 2);

		ck_map_create_object(fid, tile);
		return 0;
	}



    const luaL_Reg map_lib[] = {
		{ "get_id", l_get_map_id },
		{ "add_scenery", l_add_scenery },
		{ "add_tile", l_add_tile },
		{ "set_camera_borders", l_set_camera_borders },
		{ "remove_blocker", l_remove_blocker },
		{ "create_blocker", l_create_blocker },
		{ "create_object", l_create_object },
		{ "create_object_fid", l_create_object },
        { nullptr,          nullptr }
    };

}

int luaopen_ck_map(lua_State* L) {
    luaL_newlib(L, map_lib);
    return 1;
}
