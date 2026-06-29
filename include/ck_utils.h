#include <iostream>
extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}
#include "ck_log.h"

extern lua_State* gLuaState;

static bool safe_pcall_with_traceback(lua_State* L, int nargs = 0, int nresults = 0) {
	int func_idx = lua_gettop(L) - nargs;
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "traceback");
	lua_remove(L, -2);
	lua_insert(L, func_idx);

	int err_handler_idx = func_idx;
	int status = lua_pcall(L, nargs, nresults, err_handler_idx);

	lua_remove(L, err_handler_idx);

	return status == LUA_OK;
}

inline void ck_create_global_subtable(const char* global_name, const char* sub_table_name) {
    if (gLuaState == nullptr) return;

    lua_getglobal(gLuaState, global_name);
    if (lua_istable(gLuaState, -1)) {
        lua_pushstring(gLuaState, sub_table_name);
        lua_newtable(gLuaState);
        lua_settable(gLuaState, -3);
    }
    lua_pop(gLuaState, 1);
}
