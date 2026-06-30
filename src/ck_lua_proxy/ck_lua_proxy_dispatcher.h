#ifndef CK_LUA_PROXY_DISPATCHER_H
#define CK_LUA_PROXY_DISPATCHER_H

#include "ck_lua_proxy/ck_lua_proxy.h"
#include <string>

namespace ck::proxy {
	void on_map_update(int ticks);

	bool proxy_emit_start(const char* mod_id, const char* event_name);
	void proxy_emit_end(const char* mod_id, const char* event_name, int total_args);

	template<typename... Args>
	void emit_for_mod(const char* mod_id, const char* event_name, Args... args) {
		LuaStackGuard stack_guard;

		if (!proxy_emit_start(mod_id, event_name)) return;

		(push_arg(args), ...);

		int total_args = 2 + sizeof...(Args);
		proxy_emit_end(mod_id, event_name, total_args);
	}

	bool on_proc(int lua_id, int proc_id, const char* object_mod_id);
    bool load_mod(const char* mod_id);

	void clear_tracked_objects();
	void clear_registry();
}

#endif
