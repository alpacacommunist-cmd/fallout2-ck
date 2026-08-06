#include <cstring>

#include "ck_scripting.h"
#include "ck_utils.h"
#include "ck_encoding.h"

#include "ck_registry/ck_registry.h"
#include "ck_proto/ck_proto_cache.h"
#include "ck_proto/ck_proto_registry.h"

#include "ck_state/ck_state.h"
#include "ck_dispatcher/ck_dispatcher.h"
#include "ck_i18n/ck_i18n.h"

#include <lua.hpp>
#include "ck_lua_proxy/ck_lua_proxy.h"
lua_State* gLuaState = nullptr;

#include "settings.h"

#include "ck_log.h"
static const Logger logger("CK Scripting");


// l_ck_monitor_print -> ckMonitorPrint -> fallout2.monitor.print
int l_ck_monitor_print(lua_State* L) {
	const char* message = luaL_checkstring(L, 1);

	if (message != nullptr) fallout::displayMonitorAddMessage(utf8_to_cp1251(message).c_str());

	return 0;
}

// ck scripting reload mods
void ck_reload_mods() {
	logger.info("ck_reload_mods");
    if (gLuaState == nullptr) {
		logger.error("Cannot reload mods: Lua state is null");
        return;
    }

    lua_getglobal(gLuaState, "ckReloadMods");

    if (!lua_isfunction(gLuaState, -1)) {
		logger.error("ckReloadMods() is not defined");
        lua_pop(gLuaState, 1);
        return;
    }

    if (lua_pcall(gLuaState, 0, 0, 0) != LUA_OK) {
		logger.error("Reload Error: {}", lua_tostring(gLuaState, -1));

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

static bool is_test_mode           = false;
static std::string g_test_suite_name = std::string();

static int    g_game_argc = 0;
static char** g_game_argv = nullptr;
void ck_scripting_init(int argc, char** argv) {
    logger.info("Initializing LuaJIT backend...");

    g_game_argc = argc; g_game_argv = argv;

    for (int index = 1; index < g_game_argc; index++) {
        std::string arg = g_game_argv[index];
        if (arg == "--test" || arg == "--integration-tests") {
            is_test_mode = true;

            if (index + 1 < g_game_argc) g_test_suite_name = g_game_argv[index + 1];
            break;
        }
    }

    if (is_test_mode) {
        logger.info("LAUNCHING IN INTEGRATION TEST MODE: {}", g_test_suite_name);
    }

    gLuaState = luaL_newstate();
    if (!gLuaState) {
        logger.error("Failed to initialize LuaJIT state!");
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
        logger.error("Failed to load bootstrap.lua (Syntax Error):\n{}", lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
        return;
    }

    if (!safe_pcall_with_traceback(gLuaState, 0, 0)) {
        logger.error("Bootstrap Runtime Error:\n{}", lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
        return;
    }

    ck_lua_proxy_init();

    lua_getglobal(gLuaState, "ckBootstrapMods");
    if (!lua_isfunction(gLuaState, -1)) {
        logger.error("Global function 'ckBootstrapMods' not found!");
        lua_pop(gLuaState, 1);
        return;
    }

    if (!safe_pcall_with_traceback(gLuaState, 0, 0)) {
        logger.error("Runtime error during mod bootstrapping:\n{}", lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
    }
}

void ck_on_scripts_reset() {
	logger.info("ck_on_scripts_reset");
}

// Exit
void ck_scripting_exit() {
	ck_lua_proxy_shutdown();

    if (gLuaState != nullptr) {
        logger.info("ck_scripting_exit");
        logger.info("Shutting down LuaJIT backend...");
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

    logger.info("System language: {}", fallout::settings.system.language);
    ck::i18n::load_language(fallout::settings.system.language);

    lua_pushstring(gLuaState, fallout::settings.system.language.c_str());
    lua_pcall(gLuaState, 1, 0, 0);
}

// this is called from fallout2-ce once interface is ready
void ck_scripting_on_game_start() {
	ck_dispatcher_on_game_start();
}

void ck_scripting_on_engine_ready() {
	ck_scripting_set_language();
    logger.debug("ck_scripting_on_engine_ready");

    if (is_test_mode) {
		fallout::settings.ui.skip_opening_movies = 1;
	}

	gProtoCache.initialize("build/proto_cache.db");

    ck_dispatcher_on_engine_ready();
}

void ck_scripting_on_object_destroyed(fallout::Object* object) {
	ck::registry::created::remove_by_ptr(object);
}

// loadsave.cc
void ck_scripting_on_before_game_load(const char* path) {
	logger.debug("ck_scripting_on_before_game_load");

	ck::registry::clear();

	ck_state_load(path);
}

void ck_scripting_on_game_loaded() {
	ck_dispatcher_on_game_loaded();

	ck::on_map_enter();
}

void ck_scripting_on_before_game_save() {
	logger.debug("on_before_game_save");

	ck::registry::deleted::unhide();
	ck::registry::modified::restore_sids();
    // ck::proto::convert_custom_items_to_base(fallout::gDude);
}

void ck_scripting_on_game_save(const char* path) {
    logger.debug("on_game_save");

	ck_state_save(path);

	ck::registry::deleted::hide();
	ck::registry::modified::reapply_sids();
    // ck::proto::restore_custom_items_from_base(fallout::gDude);
}

void ck_scripting_load_game_slot(int slot) {
	fallout::ck_load_game_slot(slot);
}

const char* ck_testing_get_current_suite() {
    return g_test_suite_name.c_str();
}

void ck_testing_set_current_suite(const char* name) {
    g_test_suite_name = std::string(name);
}
