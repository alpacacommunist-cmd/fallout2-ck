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
    void on_map_update(int ticks) {
		execute_proxy_call<bool>(detail::on_map_update, ticks);
    }

	void emit_event(const char* mod_id, const char* event_name, std::initializer_list<ProxyArg> args) {
		if (!internal_call_start(detail::emit_for_mod)) return;

		push_arg(mod_id);
		push_arg(event_name);

		for (const auto& arg : args) std::visit([](const auto& val) { push_arg(val); }, arg);
		int total_args = 2 + args.size();

		if (!internal_call_execute(total_args, 0)) {
			std::string error_msg = lua_isstring(gLuaState, -1) ? lua_tostring(gLuaState, -1) : "Unknown error";
			log.error("Error routing event '{}:{}': {}", mod_id, event_name, error_msg);
			lua_pop(gLuaState, 1);
		}
	}

    bool on_proc(int lua_id, int proc_id, int fixed_param, const char* object_mod_id) {
        return execute_proxy_call<bool>(detail::on_proc, lua_id, proc_id, fixed_param);
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
