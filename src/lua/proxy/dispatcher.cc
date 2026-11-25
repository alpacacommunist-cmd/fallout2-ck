#include "lua/proxy/proxy.h"
#include "lua/proxy/dispatcher.h"
#include "runtime/proto_registry/registry.h"
#include "runtime/mods.h"
#include <lua.hpp>

extern lua_State* gLuaState;

#include "log.h"
static constexpr logging::Context logger{"ck_lua_proxy_dispatcher.cc"};

namespace ck::proxy::detail {
    extern int emit;
    extern int emit_for_mod;
    extern int on_map_update;
    extern int on_proc;
    extern int on_proto_proc;
    extern int critter_killed;
}

namespace ck::proxy {
    static void push_arg(fallout::Object* value) {
        lua_pushlightuserdata(gLuaState, value);
    }

    void on_map_update(int ticks) {
		execute_proxy_call<bool>(detail::on_map_update, ticks);
    }

    void emit_global(const char* event_name, std::initializer_list<ProxyArg> args) {
		if (!internal_call_start(detail::emit)) return;

        push_arg(event_name);
		for (const auto& arg : args) std::visit([](const auto& val) { push_arg(val); }, arg);
		int total_args = 1 + args.size();

		if (!internal_call_execute(total_args, 0)) {
			std::string error_msg = lua_isstring(gLuaState, -1) ? lua_tostring(gLuaState, -1) : "Unknown error";
			logger.error("Error routing event '{}': {}", event_name, error_msg);
			lua_pop(gLuaState, 1);
		}
    }

	void emit_event(const char* mod_id, const char* event_name, std::initializer_list<ProxyArg> args) {
		if (!internal_call_start(detail::emit_for_mod)) return;

		push_arg(mod_id);
		push_arg(event_name);

		for (const auto& arg : args) std::visit([](const auto& val) { push_arg(val); }, arg);
		int total_args = 2 + args.size();

		if (!internal_call_execute(total_args, 0)) {
			std::string error_msg = lua_isstring(gLuaState, -1) ? lua_tostring(gLuaState, -1) : "Unknown error";
			logger.error("Error routing event '{}:{}': {}", mod_id, event_name, error_msg);
			lua_pop(gLuaState, 1);
		}
	}

    bool on_proc(int lua_id, int proc_id, int param, const char* object_mod_id) {
        return execute_proxy_call<bool>(detail::on_proc, lua_id, proc_id, param, object_mod_id);
    }

    bool on_proto_proc(int proc_id, int param, fallout::Object* object) {
        return execute_proxy_call<bool>(detail::on_proto_proc, proc_id, param, object);
    }

    bool critter_killed(fallout::Object* victim, fallout::Object* killer) {
        // In some cases there's no killer
        if (killer == nullptr) killer = victim;

        return execute_proxy_call<bool>(detail::critter_killed, victim, killer);
    }
}
