#include <iostream>
#include <cstring>
#include <algorithm>
#include <format>

#include "ck_scripting.h"
#include "ck_utils.h"
#include "ck_encoding.h"

#include "ce_config/ck_config_patch.h"
#include "ce_config/ck_message_patch.h"
#include "ce_config/ck_map_registry.h"
#include "ce_config/ck_map_patch.h"

// bindings
#include "ck_assets/assets_bindings.h"

#include "game_time/ck_game_time.h"
#include "object/ck_object_registry.h"

#include "ck_assets/ck_frm.h"
#include "ck_assets/ck_asset_registry.h"
#include "ck_assets/ck_proto_cache.h"

#include "ck_state/ck_state.h"
#include "ck_dispatcher/ck_dispatcher.h"

#include "display_monitor.h"
#include "settings.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
#include "../../src/vendor/luajit/src/lualib.h"
#include "../../src/vendor/luajit/src/lauxlib.h"
}

#include "ck_log.h"
static const Logger log("CK Scripting");

CkAssetRegistry gAssetRegistry;
CkMapRegistry gMapRegistry;

// lua state global pointer, lives as long as game lives
lua_State* gLuaState = nullptr;

static void ck_requiref(lua_State* L, const char* modname, lua_CFunction openf, int gl) {
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname); // push modname as argument for openf
    // call openf(modname). returns module table on stck
    lua_call(L, 1, 1);

	// package.loaded
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "loaded"); // stack: [module, package, loaded]

	// package.loaded[modname] = module
	lua_pushvalue(L, -3);
	lua_setfield(L, -2, modname);

	// -package -loaded
	lua_pop(L, 2); // stack: [module]

	// if gl == true register module as global in ck
	if (gl) {
		lua_getglobal(L, "ck"); // stack: [module, ck]

		// name after dot
		const char* dot = strchr(modname, '.');
		const char* subname = dot ? dot + 1 : modname;

		lua_pushvalue(L, -2); // copy module on top of stack
		lua_setfield(L, -2, subname); // ck[subname] = module

		lua_pop(L, 1); // -ck
	}
}

void ck_scripting_register_location(const std::string& modId, const std::string& mapsDir,
        const std::string& name, const std::string& subName, const std::string& mapFile,
        const std::string& music, int worldX, int worldY, const std::string& size,
        int entranceX, int entranceY, int entranceTile) {

    static int nextMapIdx  = -1;
    static int nextAreaIdx = -1;
    static bool registryLoaded = false;

    if (!registryLoaded) {
        gMapRegistry.load("../ck_registry.json");
        registryLoaded = true;
    }

    if (nextMapIdx == -1) {
        nextMapIdx  = ck_config_next_map_index("data\\data\\maps.txt");
        nextAreaIdx = ck_config_next_area_index("data\\data\\city.txt");
    }

	std::string mapFileUpper = mapFile;
	std::transform(mapFileUpper.begin(), mapFileUpper.end(), mapFileUpper.begin(), ::toupper); // "TSTCV"

	std::string mapFileLower = mapFile;
	std::transform(mapFileLower.begin(), mapFileLower.end(), mapFileLower.begin(), ::tolower); // "tstcv"

	std::string registryKey = modId + ":" + mapFileUpper;
	bool map_entry_is_new = !gMapRegistry.has(registryKey);

	auto& entry = gMapRegistry.resolve(registryKey, nextMapIdx, nextAreaIdx);
	if (map_entry_is_new) {
		nextMapIdx++;
		nextAreaIdx++;
	}

    int mapIdx  = entry.mapIdx;
    int areaIdx = entry.areaIdx;

    std::string mapSection  = "Map "  + std::to_string(mapIdx);
    std::string areaSection = "Area " + std::to_string(areaIdx);

	log.info("Registering location: {} as {} / {}", name, mapSection, areaSection);

    // maps.txt
    ck_config_patch_add("data\\maps.txt", mapSection, "lookup_name", name);
    ck_config_patch_add("data\\maps.txt", mapSection, "map_name",    mapFileUpper);
    ck_config_patch_add("data\\maps.txt", mapSection, "music",       music);
    ck_config_patch_add("data\\maps.txt", mapSection, "saved",       "Yes");

    // city.txt
	std::string worldPos = std::format("{},{}", worldX, worldY);
	std::string entrance = std::format("On,{},{},{},-1,{},0", entranceX, entranceY, name, entranceTile);

    ck_config_patch_add("data\\city.txt", areaSection, "area_name",             name);
    ck_config_patch_add("data\\city.txt", areaSection, "world_pos",             worldPos);
    ck_config_patch_add("data\\city.txt", areaSection, "start_state",           "On");
    ck_config_patch_add("data\\city.txt", areaSection, "size",                  size);
    ck_config_patch_add("data\\city.txt", areaSection, "townmap_art_idx",       "-1");
    ck_config_patch_add("data\\city.txt", areaSection, "townmap_label_art_idx", "-1");
	ck_config_patch_add("data\\city.txt", areaSection, "entrance_0",            entrance);

	// map.msg — city name
	ck_message_patch_add("game/map.msg", 1500 + areaIdx, name);

	int mapMsgBase = (mapIdx * 3) + 100;
	ck_message_patch_add("game/map.msg", mapMsgBase, name);
	// (mapIdx * 3) + 101 -> first line save
	ck_message_patch_add("game/map.msg", mapMsgBase + 1, name);
	// (mapIdx * 3) + 200 -> description
	ck_message_patch_add("game/map.msg", (mapIdx * 3) + 200, subName);

    // mod map file path
	std::string mapFilePath = std::format("../{}/{}.MAP", mapsDir, mapFileUpper);

    // only patch header once
    if (map_entry_is_new) {
		ck_map_patch_header(mapFilePath, mapFileUpper + ".MAP", mapIdx);
    }

    // path for mapBuildPath hook
    ck_map_register_path(mapFileLower, mapFilePath);

    // save registry after each registration
    gMapRegistry.save("../ck_registry.json");
}

void ck_call_hook_int(const char* name, int arg);

// l_ck_monitor_print -> ckMonitorPrint -> fallout2.monitor.print
int l_ck_monitor_print(lua_State* L) {
	const char* message = luaL_checkstring(L, 1);

	if (message != nullptr) fallout::displayMonitorAddMessage(utf8_to_cp1251(message).c_str());

	return 0;
}

static int l_ck_register_location(lua_State* L) {
	const char* modId    = luaL_checkstring(L, 1);
	const char* mapsDir  = luaL_checkstring(L, 2);
    const char* name         = luaL_checkstring(L, 3);
	const char* subName     = luaL_checkstring(L, 4);
    const char* mapFile      = luaL_checkstring(L, 5);
    const char* music        = luaL_checkstring(L, 6);
    int worldX               = luaL_checkinteger(L, 7);
    int worldY               = luaL_checkinteger(L, 8);
    const char* size         = luaL_checkstring(L, 9);
    int entranceX            = luaL_checkinteger(L, 10);
    int entranceY            = luaL_checkinteger(L, 11);
    int entranceTile         = luaL_checkinteger(L, 12);

    ck_scripting_register_location(modId, mapsDir, name, subName, mapFile, music, worldX, worldY,
			size, entranceX, entranceY, entranceTile);

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
	gObjectRegistry.destroy_objects_for_mod(target_mod_id);
}

// Init
void ck_scripting_init() {
    log.info("Initializing LuaJIT backend...");

	gLuaState = luaL_newstate();
	if (gLuaState != nullptr) {
		// Init global lua state
		luaL_openlibs(gLuaState);

		// create global ck module
		lua_newtable(gLuaState);               // stack: [ck]
		lua_setglobal(gLuaState, "ck");        // stack: []

		// ffi
		ck_create_global_subtable("ck", "rendering");
		ck_create_global_subtable("ck", "game_time");
		ck_create_global_subtable("ck", "dialogue");
		ck_create_global_subtable("ck", "map");
		// ffi end

		ck_requiref(gLuaState, "ck.assets", luaopen_ck_assets, 1);
		lua_pop(gLuaState, 1);

        // bindings. registers c <-> lua functions
		lua_register(gLuaState, "ckMonitorPrint", l_ck_monitor_print);
		lua_register(gLuaState, "ckRegisterLocation", l_ck_register_location);

        // bootstrap
		int status = luaL_loadfile(gLuaState, "../ck/system/bootstrap.lua");

		if (status == LUA_OK) {
			lua_getglobal(gLuaState, "debug");
			lua_getfield(gLuaState, -1, "traceback");
			lua_remove(gLuaState, -2);

			int err_handler_idx = lua_gettop(gLuaState) - 1;
			lua_insert(gLuaState, err_handler_idx);

			status = lua_pcall(gLuaState, 0, LUA_MULTRET, err_handler_idx);

			if (status != LUA_OK) {
				log.error("Bootstrap Error:\n{}", lua_tostring(gLuaState, -1));
				lua_pop(gLuaState, 2);
			} else {
				ck_dispatcher_init(gLuaState);
				lua_pop(gLuaState, 1);
			}
		} else {
			log.error("Failed to load bootstrap.lua (Syntax Error):\n{}", lua_tostring(gLuaState, -1));
			lua_pop(gLuaState, 1);
		}
    } else {
        std::cerr << "[CK] Failed to initialize LuaJIT state!" << std::endl;
	}
}

void ck_on_scripts_reset() {
	log.info("ck_on_scripts_reset");
}

// Exit
void ck_scripting_exit() {
	ck_dispatcher_shutdown();
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
	gObjectRegistry.remove_by_ptr(object);
}
void ck_registry_clear() { gObjectRegistry.clear(); }

// loadsave.cc
void ck_scripting_on_game_loaded() {
	ck_dispatcher_on_game_loaded();
}

// loadsave.cc
void ck_scripting_on_before_game_load(const char* path) {
	log.debug("ck_scripting_on_before_game_load");

	ck_state_load(path);
}

// loadsave.cc
void ck_scripting_on_game_save(const char* path) {
	log.debug("ck_scripting_on_game_save");

	ck_state_save(path);
}

// loadsave.cc

int ck_get_config_int(const char* key, int default_value) {
    if (gLuaState == nullptr) return default_value;

    // Search our global LUA function for managing configs
    lua_getglobal(gLuaState, "ckOnGetConfig");

    if (!lua_isfunction(gLuaState, -1)) {
        lua_pop(gLuaState, 1); // clear stack if function not found
        return default_value;
    }

    // Push arguments to stack
    lua_pushstring(gLuaState, key);
    lua_pushinteger(gLuaState, default_value);

    // call function: 2 arguments, 1 return
    int status = lua_pcall(gLuaState, 2, 1, 0);
    if (status != 0) {
        std::cerr << "[CK] Config Error (Int): " << lua_tostring(gLuaState, -1) << std::endl;
        lua_pop(gLuaState, 1); // clears out an error
        return default_value;
    }

    // Grab the result from stack
    int result = default_value;
    if (lua_isnumber(gLuaState, -1)) {
        result = (int)lua_tointeger(gLuaState, -1);
		log.info("Received new value for key {}: {}", key, result);
    }

    // clear out the stack
    lua_pop(gLuaState, 1);

    return result;
}
