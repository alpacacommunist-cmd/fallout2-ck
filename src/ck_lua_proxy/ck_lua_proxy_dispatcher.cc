#include "ck_lua_proxy/ck_lua_proxy.h"
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
	// ck_lua_proxy.cc
	extern bool safe_pcall_with_traceback(lua_State* L, int nargs, int nresults);

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
		if (!safe_pcall_with_traceback(gLuaState, total_args, 0)) {
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

	void clear_tracked_objects() {
		execute_proxy_call<bool>(detail::clear_tracked_objects);
	}
	void clear_registry() {
		execute_proxy_call<bool>(detail::clear_registry);
	}
}
