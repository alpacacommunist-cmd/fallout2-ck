#include "ck_lua_proxy/ck_lua_proxy_dispatcher.h"
#include <lua.hpp>

extern lua_State* gLuaState;

#include "ck_log.h"
static const Logger log("CK Dispatcher Lua Proxy");

namespace ck::proxy {

    void execute_map_update(int ticks) {
        if (ck::proxy::on_map_update == LUA_NOREF) return;

        lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, ck::proxy::on_map_update);
        lua_pushinteger(gLuaState, ticks);

        if (lua_pcall(gLuaState, 1, 0, 0) != LUA_OK) {
            log.error("Error in ck_on_map_update: {}", lua_tostring(gLuaState, -1));
            lua_pop(gLuaState, 1);
        }
    }

	// emit for mod
	bool proxy_emit_start(const char* mod_id, const char* event_name) {
		if (!gLuaState || ck::proxy::emit_for_mod == LUA_NOREF) return false;

		lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, ck::proxy::emit_for_mod);
		lua_pushstring(gLuaState, mod_id);
		lua_pushstring(gLuaState, event_name);
		return true;
	}

    void proxy_push_arg(int val)               { lua_pushinteger(gLuaState, val); }
    void proxy_push_arg(unsigned int val)      { lua_pushinteger(gLuaState, val); }
    void proxy_push_arg(double val)            { lua_pushnumber(gLuaState, val); }
    void proxy_push_arg(const char* val)       { lua_pushstring(gLuaState, val); }
    void proxy_push_arg(const std::string& val) { lua_pushstring(gLuaState, val.c_str()); }
    void proxy_push_arg(bool val)              { lua_pushboolean(gLuaState, val); }

    void proxy_emit_end(const char* mod_id, const char* event_name, int total_args) {
        if (lua_pcall(gLuaState, total_args, 0, 0) != LUA_OK) {
            log.error("Error routing event '{}:{}': {}", mod_id, event_name, lua_tostring(gLuaState, -1));
            lua_pop(gLuaState, 1);
        }
    }
	// emit for mod end

    bool execute_on_proc(int lua_id, int proc_id, const char* object_mod_id) {
        return execute_proxy_call<bool>(ck::proxy::on_proc, lua_id, proc_id);
    }

    bool execute_load_mod(const char* mod_id) {
        return execute_proxy_call<bool>(ck::proxy::load_and_init_mod, mod_id);
    }

    void execute_clear_registry_fn(int func_ref) {
        execute_proxy_call<bool>(func_ref);
    }
}
