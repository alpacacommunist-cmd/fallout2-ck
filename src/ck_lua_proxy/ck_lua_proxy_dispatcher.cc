#include "ck_lua_proxy/ck_lua_proxy_dispatcher.h"
#include <lua.hpp>

extern lua_State* gLuaState;

#include "ck_log.h"
static const Logger log("CK Dispatcher Lua Proxy");

namespace ck::proxy::detail {
    extern int emit_for_mod;
    extern int on_map_update;
    extern int on_proc;
    extern int clear_tracked_objects;
    extern int clear_registry;
    extern int load_and_init_mod;
}

namespace ck::proxy {

    void on_map_update(int ticks) {
		execute_proxy_call<bool>(detail::on_map_update, ticks);
    }

	// emit for mod
	bool proxy_emit_start(const char* mod_id, const char* event_name) {
		if (!internal_call_start(detail::emit_for_mod)) return false;
        push_arg(mod_id);
        push_arg(event_name);
        return true;
	}

    void proxy_emit_end(const char* mod_id, const char* event_name, int total_args) {
        if (lua_pcall(gLuaState, total_args, 0, 0) != LUA_OK) {
            log.error("Error routing event '{}:{}': {}", mod_id, event_name, lua_tostring(gLuaState, -1));
            lua_pop(gLuaState, 1);
        }
    }
	// emit for mod end

    bool on_proc(int lua_id, int proc_id, const char* object_mod_id) {
        return execute_proxy_call<bool>(detail::on_proc, lua_id, proc_id);
    }

    bool load_mod(const char* mod_id) {
        return execute_proxy_call<bool>(detail::load_and_init_mod, mod_id);
    }

    void clear_registry_fn(int func_ref) {
        execute_proxy_call<bool>(func_ref);
    }
}
