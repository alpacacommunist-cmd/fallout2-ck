#include <cstring>
#include <algorithm>
#include <format>

#include "ck_scripting.h"
#include "ck_utils.h"
#include "ck_encoding.h"

#include "ck_messages/ck_messages.h"

#include "ck_registry/ck_registry.h"

#include "ck_assets/ck_frm.h"
#include "ck_assets/ck_asset_registry.h"
#include "ck_assets/ck_proto_cache.h"

#include "ck_state/ck_state.h"
#include "ck_dispatcher/ck_dispatcher.h"

#include <lua.hpp>
#include "ck_lua_proxy/ck_lua_proxy.h"
lua_State* gLuaState = nullptr;

#include "settings.h"

#include "ck_log.h"
static const Logger log("CK Scripting");

CkAssetRegistry gAssetRegistry;

// l_ck_monitor_print -> ckMonitorPrint -> fallout2.monitor.print
int l_ck_monitor_print(lua_State* L) {
	const char* message = luaL_checkstring(L, 1);

	if (message != nullptr) fallout::displayMonitorAddMessage(utf8_to_cp1251(message).c_str());

	return 0;
}

// ck scripting reload mods
void ck_reload_mods() {
	log.info("ck_reload_mods");
    if (gLuaState == nullptr) {
		log.error("Cannot reload mods: Lua state is null");
        return;
    }

    lua_getglobal(gLuaState, "ckReloadMods");

    if (!lua_isfunction(gLuaState, -1)) {
		log.error("ckReloadMods() is not defined");
        lua_pop(gLuaState, 1);
        return;
    }

    if (lua_pcall(gLuaState, 0, 0, 0) != LUA_OK) {
		log.error("Reload Error: {}", lua_tostring(gLuaState, -1));

        lua_pop(gLuaState, 1);
		return;
    }
}

void ck_registry_destroy_objects_for_mod(const char* target_mod_id) {
	ck::registry::clear_resources_for_mod(target_mod_id);
}

static const luaL_Reg CK_GLOBAL_FUNCTIONS[] = {
    {"ckMonitorPrint",     l_ck_monitor_print},
    {nullptr,              nullptr}
};

// Init

void ck_scripting_init() {
    log.info("Initializing LuaJIT backend...");

    gLuaState = luaL_newstate();
    if (!gLuaState) {
        log.error("Failed to initialize LuaJIT state!");
        return;
    }

    luaL_openlibs(gLuaState);

    lua_newtable(gLuaState);
    lua_setglobal(gLuaState, "ck");

    for (const char* subtable : {"assets", "rendering", "game_time", "dialogue", "map"}) {
        ck_create_global_subtable("ck", subtable);
    }

	lua_pushvalue(gLuaState, LUA_GLOBALSINDEX);
    luaL_setfuncs(gLuaState, CK_GLOBAL_FUNCTIONS, 0);
    lua_pop(gLuaState, 1);

    if (luaL_loadfile(gLuaState, "../ck/system/bootstrap.lua") != LUA_OK) {
        log.error("Failed to load bootstrap.lua (Syntax Error):\n{}", lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
        return;
    }

    if (!safe_pcall_with_traceback(gLuaState, 0, 0)) {
        log.error("Bootstrap Runtime Error:\n{}", lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
        return;
    }

    ck_lua_proxy_init();

    lua_getglobal(gLuaState, "ckBootstrapMods");
    if (!lua_isfunction(gLuaState, -1)) {
        log.error("Global function 'ckBootstrapMods' not found!");
        lua_pop(gLuaState, 1);
        return;
    }

    if (!safe_pcall_with_traceback(gLuaState, 0, 0)) {
        log.error("Runtime error during mod bootstrapping:\n{}", lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
    }
}

void ck_on_scripts_reset() {
	log.info("ck_on_scripts_reset");
}

// Exit
void ck_scripting_exit() {
	ck_lua_proxy_shutdown();

    if (gLuaState != nullptr) {
        log.info("ck_scripting_exit");
        log.info("Shutting down LuaJIT backend...");
        lua_close(gLuaState);
        gLuaState = nullptr;
    }
}

void ck_scripting_set_language() {
    if (gLuaState == nullptr) return;

    lua_getglobal(gLuaState, "ckSetLanguage");
    if (!lua_isfunction(gLuaState, -1)) {
        lua_pop(gLuaState, 1);
        return;
    }

    log.info("System language: {}", fallout::settings.system.language);

    lua_pushstring(gLuaState, fallout::settings.system.language.c_str());
    lua_pcall(gLuaState, 1, 0, 0);
}

// this is called from fallout2-ce once interface is ready
void ck_scripting_on_game_start() {
	ck_dispatcher_on_game_start();
}

void ck_scripting_on_engine_ready() {
	ck_scripting_set_language();
    log.debug("ck_scripting_on_engine_ready");

#ifdef USE_PROTO_CACHE
	gProtoCache.initialize("build/proto_cache.db");
#else
	log.debug("Proto Cache is disabled in this build");
#endif
}

void ck_scripting_on_object_destroyed(fallout::Object* object) {
	ck::registry::created::remove_by_ptr(object);
}

// loadsave.cc
void ck_scripting_on_before_game_load(const char* path) {
	log.debug("ck_scripting_on_before_game_load");

	ck::registry::clear();
	ck_state_load(path);
}

void ck_scripting_on_game_loaded() {
	ck_dispatcher_on_game_loaded();
}

void ck_scripting_on_before_game_save() {
	log.debug("on_before_game_save");

	ck::registry::deleted::unhide();
}

void ck_scripting_on_game_save(const char* path) {
    log.debug("on_game_save");

	ck_state_save(path);
	ck::registry::deleted::hide();
}
