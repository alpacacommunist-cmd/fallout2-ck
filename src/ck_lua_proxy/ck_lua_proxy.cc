#include "ck_lua_proxy/ck_lua_proxy.h"

#include <lua.hpp>
#include <array>

#include "ck_log.h"
static const Logger log("CK Lua Proxy");

extern lua_State* gLuaState;
extern const char* g_current_mod_id;

struct LuaHookBinding { std::string_view module_name; std::string_view function_name; int* target_ref; };
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
    using namespace ck::proxy::detail;

	const std::array<LuaHookBinding, 10> hooks = {{
		{ "ck.system.events",    "emit_for_mod",          &emit_for_mod },
		{ "ck.system.events",    "on_map_update",         &on_map_update },
		{ "ck.system.events",    "on_proc",               &on_proc },
		{ "ck.fallout2.state",   "clear_tracked_objects", &clear_tracked_objects },
		{ "ck.fallout2.objects", "clear_registry",        &clear_registry },
		{ "ck.system.loader",    "load_and_init_mod",     &load_and_init_mod },
		{ "ck.fallout2.state",   "get_state_tile",        &get_state_tile },
		{ "ck.fallout2.state",   "get_state_data",        &get_state_data },
		{ "ck.fallout2.state",   "sync_load",             &state_sync_load },
		{ "ck.fallout2.state",   "sync_save",             &state_sync_save }
	}};

	for (const auto& hook : hooks) {
		*hook.target_ref = cache_module_function(hook.module_name.data(), hook.function_name.data());

		if (*hook.target_ref == LUA_NOREF) log.error("Can't cache ref: {}.{}", hook.module_name, hook.function_name);
	}

    log.info("successfully initialized and cached Lua hooks.");
}

void ck_lua_proxy_shutdown() {
    using namespace ck::proxy::detail;

	    const std::array<int*, 10> refs = {
        &emit_for_mod,
        &on_map_update,
        &on_proc,
        &clear_tracked_objects,
        &clear_registry,
        &load_and_init_mod,
        &get_state_tile,
        &get_state_data,
        &state_sync_load,
        &state_sync_save
    };

    for (int* ref_ptr : refs) {
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
        if (gLuaState) {
            lua_settop(gLuaState, initial_top);
        }
    }

    bool is_ready() { return gLuaState != nullptr; }

	void push_arg(int val)               { lua_pushinteger(gLuaState, val); }
	void push_arg(unsigned int val)      { lua_pushinteger(gLuaState, val); }
	void push_arg(double val)            { lua_pushnumber(gLuaState, val); }
	void push_arg(const char* val)       { lua_pushstring(gLuaState, val); }
	void push_arg(const std::string& val) { lua_pushstring(gLuaState, val.c_str()); }
	void push_arg(bool val)              { lua_pushboolean(gLuaState, val); }

	bool safe_pcall_with_traceback(lua_State* L, int nargs, int nresults) {
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

	bool internal_call_start(int func_ref) {
		if (!gLuaState || func_ref == LUA_NOREF) return false;
		lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, func_ref);
		return true;
	}

	bool internal_call_execute(int nargs, int nresults) {
		if (!safe_pcall_with_traceback(gLuaState, nargs, nresults)) {
			log.error("Runtime error during Lua proxy execution");
			lua_pop(gLuaState, 1);
			return false;
		}
		return true;
	}

	int internal_pop_int() {
		int res = static_cast<int>(lua_tointeger(gLuaState, -1));
		lua_pop(gLuaState, 1);
		return res;
	}

	bool internal_pop_bool() {
		bool res = lua_toboolean(gLuaState, -1);
		lua_pop(gLuaState, 1);
		return res;
	}

	std::string internal_pop_string() {
		std::string res = lua_isstring(gLuaState, -1) ? lua_tostring(gLuaState, -1) : "";
		lua_pop(gLuaState, 1);
		return res;
	}

	int read_table_int(const char* key, int default_val) {
		if (!gLuaState || !lua_istable(gLuaState, -1)) return default_val;

		int res = default_val;
		lua_getfield(gLuaState, -1, key);
		if (lua_isnumber(gLuaState, -1)) res = static_cast<int>(lua_tointeger(gLuaState, -1));
		lua_pop(gLuaState, 1);
		return res;
	}

	bool read_table_bool(const char* key, bool default_val) {
		if (!gLuaState || !lua_istable(gLuaState, -1)) return default_val;

		bool res = default_val;
		lua_getfield(gLuaState, -1, key);
		if (lua_isboolean(gLuaState, -1)) res = lua_toboolean(gLuaState, -1);
		lua_pop(gLuaState, 1);
		return res;
	}

	std::string read_table_string(const char* key, const std::string& default_val) {
		if (!gLuaState || !lua_istable(gLuaState, -1)) return default_val;

		std::string res = default_val;
		lua_getfield(gLuaState, -1, key);
		if (lua_isstring(gLuaState, -1)) res = lua_tostring(gLuaState, -1);
		lua_pop(gLuaState, 1);
		return res;
	}
}
