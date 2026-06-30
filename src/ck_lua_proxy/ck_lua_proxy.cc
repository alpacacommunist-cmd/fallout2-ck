#include "ck_lua_proxy/ck_lua_proxy.h"

#include <lua.hpp>

#include "ck_log.h"
static const Logger log("CK Dispatcher");

extern lua_State* gLuaState;
extern const char* g_current_mod_id;

namespace ck::proxy {
    int emit_for_mod          = LUA_NOREF;
    int on_map_update         = LUA_NOREF;
    int on_proc               = LUA_NOREF;
    int clear_tracked_objects = LUA_NOREF;
    int clear_registry        = LUA_NOREF;
    int load_and_init_mod     = LUA_NOREF;
    int get_state_tile        = LUA_NOREF;
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
    using namespace ck::proxy;

    load_and_init_mod     = cache_module_function("ck.system.loader", "load_and_init_mod");
    emit_for_mod          = cache_module_function("ck.system.events", "emit_for_mod");
    on_map_update         = cache_module_function("ck.system.events", "ck_on_map_update");
    on_proc               = cache_module_function("ck.system.events", "ck_on_proc");
    clear_tracked_objects = cache_module_function("ck.fallout2.state",  "clear_tracked_objects");
    get_state_tile        = cache_module_function("ck.fallout2.state", "get_state_tile");
    state_sync_save       = cache_module_function("ck.fallout2.state", "sync_save");
    state_sync_load       = cache_module_function("ck.fallout2.state", "sync_load");
    clear_registry        = cache_module_function("ck.fallout2.objects", "clear_registry");

    log.info("successfully initialized and cached Lua hooks.");
}

void ck_lua_proxy_shutdown() {
    using namespace ck::proxy;

    if (gLuaState) {
        if (emit_for_mod != LUA_NOREF)          luaL_unref(gLuaState, LUA_REGISTRYINDEX, emit_for_mod);
        if (on_map_update != LUA_NOREF)         luaL_unref(gLuaState, LUA_REGISTRYINDEX, on_map_update);
        if (on_proc != LUA_NOREF)               luaL_unref(gLuaState, LUA_REGISTRYINDEX, on_proc);
        if (clear_tracked_objects != LUA_NOREF) luaL_unref(gLuaState, LUA_REGISTRYINDEX, clear_tracked_objects);
        if (clear_registry != LUA_NOREF)        luaL_unref(gLuaState, LUA_REGISTRYINDEX, clear_registry);
        if (load_and_init_mod != LUA_NOREF)     luaL_unref(gLuaState, LUA_REGISTRYINDEX, load_and_init_mod);
        if (get_state_tile != LUA_NOREF)        luaL_unref(gLuaState, LUA_REGISTRYINDEX, get_state_tile);
        if (state_sync_load != LUA_NOREF)       luaL_unref(gLuaState, LUA_REGISTRYINDEX, state_sync_load);
        if (state_sync_save != LUA_NOREF)       luaL_unref(gLuaState, LUA_REGISTRYINDEX, state_sync_save);

        emit_for_mod          = LUA_NOREF;
        on_map_update         = LUA_NOREF;
        on_proc               = LUA_NOREF;
        clear_tracked_objects = LUA_NOREF;
        clear_registry        = LUA_NOREF;
        load_and_init_mod     = LUA_NOREF;
        get_state_tile        = LUA_NOREF;
        state_sync_save       = LUA_NOREF;
        state_sync_load       = LUA_NOREF;
    }
}

namespace ck::proxy {
    bool is_ready() { return gLuaState != nullptr; }

    void execute_map_update(int ticks) {
        if (ck::proxy::on_map_update == LUA_NOREF) return;

        lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, ck::proxy::on_map_update);
        lua_pushinteger(gLuaState, ticks);

        if (lua_pcall(gLuaState, 1, 0, 0) != LUA_OK) {
            log.error("Error in ck_on_map_update: {}", lua_tostring(gLuaState, -1));
            lua_pop(gLuaState, 1);
        }
    }

    int execute_get_state_tile(int map_id, const std::string& lua_tag) {
        if (ck::proxy::get_state_tile == LUA_NOREF) return -1;

        lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, ck::proxy::get_state_tile);
        lua_pushstring(gLuaState, g_current_mod_id);
        lua_pushinteger(gLuaState, map_id);
        lua_pushstring(gLuaState, lua_tag.c_str());

        int tile = -1;
        if (lua_pcall(gLuaState, 3, 1, 0) == LUA_OK) {
            if (lua_isnumber(gLuaState, -1)) {
                tile = (int)lua_tointeger(gLuaState, -1);
            }
            lua_pop(gLuaState, 1);
        } else {
            log.error("Error in get_state_tile: {}", lua_tostring(gLuaState, -1));
            lua_pop(gLuaState, 1);
        }
        return tile;
    }
}
