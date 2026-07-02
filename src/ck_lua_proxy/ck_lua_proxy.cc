#include "ck_lua_proxy/ck_lua_proxy.h"

#include <lua.hpp>
#include <array>

#include "ck_log.h"
static const Logger log("CK Lua Proxy");

extern lua_State* gLuaState;
extern const char* g_current_mod_id;

namespace ck::proxy::detail {
    int emit_for_mod          = LUA_NOREF;
    int on_map_update         = LUA_NOREF;
    int on_proc               = LUA_NOREF;
    int clear_tracked_objects = LUA_NOREF;
    int clear_registry        = LUA_NOREF;
    int load_and_init_mod     = LUA_NOREF;
    int get_state_tile        = LUA_NOREF;
    int get_state_data        = LUA_NOREF;
    int state_sync_load       = LUA_NOREF;
    int state_sync_save       = LUA_NOREF;
}

struct LuaHookBinding { std::string_view module_name; std::string_view function_name; int* target_ref; };
const std::array<LuaHookBinding, 10> hooks = {{
	{ "ck.system.events",    "emit_for_mod",          &ck::proxy::detail::emit_for_mod },
	{ "ck.system.events",    "on_map_update",         &ck::proxy::detail::on_map_update },
	{ "ck.system.events",    "on_proc",               &ck::proxy::detail::on_proc },
	{ "ck.fallout2.state",   "clear_tracked_objects", &ck::proxy::detail::clear_tracked_objects },
	{ "ck.fallout2.objects", "clear_registry",        &ck::proxy::detail::clear_registry },
	{ "ck.system.loader",    "load_and_init_mod",     &ck::proxy::detail::load_and_init_mod },
	{ "ck.fallout2.state",   "get_state_tile",        &ck::proxy::detail::get_state_tile },
	{ "ck.fallout2.state",   "get_state_data",        &ck::proxy::detail::get_state_data },
	{ "ck.fallout2.state",   "sync_load",             &ck::proxy::detail::state_sync_load },
	{ "ck.fallout2.state",   "sync_save",             &ck::proxy::detail::state_sync_save }
}};

static int cache_module_function(const char* module_name, const char* function_name) {
	lua_getglobal(gLuaState, "require");
	lua_pushstring(gLuaState, module_name);

	if (lua_pcall(gLuaState, 1, 1, 0) != LUA_OK) {
		log.error("Failed to require module '{}': {}", module_name, lua_tostring(gLuaState, -1));
		lua_pop(gLuaState, 1);
		return LUA_NOREF;
	}

	lua_getfield(gLuaState, -1, function_name);
	if (!lua_isfunction(gLuaState, -1)) {
		log.error("Function '{}' not found in module '{}'!", function_name, module_name);
		lua_pop(gLuaState, 2);
		return LUA_NOREF;
	}

	int ref = luaL_ref(gLuaState, LUA_REGISTRYINDEX);

	lua_pop(gLuaState, 1);
	return ref;
}

void ck_lua_proxy_init() {
    for (const auto& hook : hooks) {
        *hook.target_ref = cache_module_function(hook.module_name.data(), hook.function_name.data());

        if (*hook.target_ref == LUA_NOREF) log.error("Can't cache ref: {}.{}", hook.module_name, hook.function_name);
    }

    log.info("successfully initialized and cached Lua hooks.");
}

void ck_lua_proxy_shutdown() {
    for (const auto& hook : hooks) {
        int* ref_ptr = hook.target_ref;

        if (*ref_ptr != LUA_NOREF) {
            luaL_unref(gLuaState, LUA_REGISTRYINDEX, *ref_ptr);
            *ref_ptr = LUA_NOREF;
        }
    }
}

namespace ck::proxy {
    LuaStackGuard::LuaStackGuard() {
        initial_top = gLuaState ? lua_gettop(gLuaState) : 0;
    }

    LuaStackGuard::~LuaStackGuard() {
        if (gLuaState) lua_settop(gLuaState, initial_top);
    }

    bool is_ready() { return gLuaState != nullptr; }

	void push_arg(int val)               { lua_pushinteger(gLuaState, val); }
	void push_arg(unsigned int val)      { lua_pushinteger(gLuaState, val); }
	void push_arg(double val)            { lua_pushnumber(gLuaState, val); }
	void push_arg(const char* val)       { lua_pushstring(gLuaState, val); }
	void push_arg(const std::string& val) { lua_pushstring(gLuaState, val.c_str()); }
	void push_arg(bool val)              { lua_pushboolean(gLuaState, val); }

	bool safe_pcall_with_traceback(int nargs, int nresults) {
		int func_idx = lua_gettop(gLuaState) - nargs;
		lua_getglobal(gLuaState, "debug");
		lua_getfield(gLuaState, -1, "traceback");
		lua_remove(gLuaState, -2);
		lua_insert(gLuaState, func_idx);

		int err_handler_idx = func_idx;
		int status = lua_pcall(gLuaState, nargs, nresults, err_handler_idx);
		lua_remove(gLuaState, err_handler_idx);
		return status == LUA_OK;
	}

	bool internal_call_start(int func_ref) {
		if (!gLuaState || func_ref == LUA_NOREF) return false;
		lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, func_ref);
		return true;
	}

	bool internal_call_execute(int nargs, int nresults) {
		if (!safe_pcall_with_traceback(nargs, nresults)) {
			std::string error_msg = "Unknown Lua error";
			if (lua_isstring(gLuaState, -1)) error_msg = lua_tostring(gLuaState, -1);

			log.error("Runtime error during Lua proxy execution:\n{}", error_msg);

			lua_pop(gLuaState, 1);
			return false;
		}

		return true;
	}

	template<typename T>
    static T internal_pop() {
        T res{};
        if constexpr (std::is_same_v<T, int>) res = static_cast<int>(lua_tointeger(gLuaState, -1));
        else if constexpr (std::is_same_v<T, bool>) res = lua_toboolean(gLuaState, -1);
        else if constexpr (std::is_same_v<T, std::string>) res = lua_isstring(gLuaState, -1) ? lua_tostring(gLuaState, -1) : "";

        lua_pop(gLuaState, 1);
        return res;
    }

	template<typename T>
	static T read_table_value(const char* key, T default_val) {
        if (!gLuaState || !lua_istable(gLuaState, -1)) return default_val;

        lua_getfield(gLuaState, -1, key);

        bool type_ok = false;
        if constexpr (std::is_same_v<T, int>)              type_ok = lua_isnumber(gLuaState, -1);
        else if constexpr (std::is_same_v<T, bool>)        type_ok = lua_isboolean(gLuaState, -1);
        else if constexpr (std::is_same_v<T, std::string>) type_ok = lua_isstring(gLuaState, -1);

        if (type_ok) return internal_pop<T>();

        lua_pop(gLuaState, 1);
        return default_val;
    }

	int         internal_pop_int()    { return internal_pop<int>(); }
	bool        internal_pop_bool()   { return internal_pop<bool>(); }
	std::string internal_pop_string() { return internal_pop<std::string>(); }

    int  read_table_int(const char* key, int default_val)   { return read_table_value<int>(key, default_val); }
    bool read_table_bool(const char* key, bool default_val) { return read_table_value<bool>(key, default_val); }
    std::string read_table_string(const char* key, const std::string& default_val) {
        return read_table_value<std::string>(key, default_val);
    }
}
