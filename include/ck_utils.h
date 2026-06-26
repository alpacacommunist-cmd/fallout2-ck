#include <iostream>
extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}
#include "ck_log.h"

extern lua_State* gLuaState;

inline void lua_push_args(lua_State*) {}

template<typename T, typename... Args>
void lua_push_args(lua_State* L, T first, Args... args) {
	if constexpr (std::is_same_v<T, int> || std::is_same_v<T, unsigned int>) {
		lua_pushinteger(L, first);
	} else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, std::string>) {
		lua_pushstring(L, std::string(first).c_str());
	} else if constexpr (std::is_same_v<T, bool>) {
		lua_pushboolean(L, first);
	} else if constexpr (std::is_floating_point_v<T>) {
		lua_pushnumber(L, first);
	}

	lua_push_args(L, args...);
}

template<typename... Args>
bool ck_call_lua_hook(const char* hook_name, Args... args) {
	if (gLuaState == nullptr) return false;

	lua_getglobal(gLuaState, "debug");
	lua_getfield(gLuaState, -1, "traceback");
	lua_remove(gLuaState, -2);
	int err_handler_idx = lua_gettop(gLuaState);

	lua_getglobal(gLuaState, hook_name);

	if (!lua_isfunction(gLuaState, -1)) {
		lua_pop(gLuaState, 2);
		return false;
	}

	int args_count = sizeof...(Args);
	lua_push_args(gLuaState, args...);

	int status = lua_pcall(gLuaState, args_count, 1, err_handler_idx);

	if (status != LUA_OK) {
		static const Logger c_log("CK Scripting");

		c_log.error("Hook Error ({}):\n{}", hook_name, lua_tostring(gLuaState, -1));

		lua_pop(gLuaState, 2);
		return false;
	}

	bool result = lua_toboolean(gLuaState, -1);
	lua_pop(gLuaState, 2);

	return result;
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
