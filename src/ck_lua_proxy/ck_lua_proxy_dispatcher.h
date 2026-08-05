#ifndef CK_LUA_PROXY_DISPATCHER_H
#define CK_LUA_PROXY_DISPATCHER_H

#include "ck_lua_proxy/ck_lua_proxy.h"
#include <string>
#include <initializer_list>
#include <variant>

struct CkObjectFFI;

namespace ck::proxy {
	void on_map_update(int ticks);

	bool proxy_emit_start(const char* mod_id, const char* event_name);
	void proxy_emit_end(const char* mod_id, const char* event_name, int total_args);

    using ProxyArg = std::variant<int, unsigned int, double, const char*, std::string, bool, const CkObjectFFI*>;
    void  emit_event(const char* mod_id, const char* event_name, std::initializer_list<ProxyArg> args);

	template<typename... Args>
	void emit_for_mod(const char* mod_id, const char* event_name, Args... args) {
		LuaStackGuard stack_guard;
		emit_event(mod_id, event_name, { ProxyArg(args)... });
	}

	bool on_proc(int lua_id, int proc_id, int fixed_param, const char* object_mod_id);
    bool load_mod(const char* mod_id);

	void clear_tracked_objects();
	void clear_registry();
	void clear_dialogs();
}

#endif
