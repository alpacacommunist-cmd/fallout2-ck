#include <iostream>
#include <cstring>

#include "ck_scripting.h"

// bindings
#include "game_time/game_time_bindings.h"
#include "rendering/rendering_bindings.h"
#include "map/map_bindings.h"

#include "game_time/ck_game_time.h"
#include "map/ck_map.h"

#include "ck_assets/ck_frm.h"
#include "ck_assets/ck_asset_registry.h"
#include "ck_assets/ck_proto_cache.h"

#include "display_monitor.h"
#include "proto_instance.h"
#include "tile.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
#include "../../src/vendor/luajit/src/lualib.h"
#include "../../src/vendor/luajit/src/lauxlib.h"
}

CkAssetRegistry gAssetRegistry;

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

void ck_call_hook(const char* name);
void ck_call_hook_int(const char* name, int arg);

void ck_call_hook(const char* name) {
    if (gLuaState == nullptr) return;
    lua_getglobal(gLuaState, name);
    if (!lua_isfunction(gLuaState, -1)) { lua_pop(gLuaState, 1); return; }
    if (lua_pcall(gLuaState, 0, 0, 0) != LUA_OK) {
        std::cerr << "[CK] Hook Error (" << name << "): " << lua_tostring(gLuaState, -1) << std::endl;
        lua_pop(gLuaState, 1);
    }
}

// bindings
// C <-> Lua contract, raw -> registered -> lua api
//
// l_ck_log_print -> ckLogPrint -> fallout2.log.print
int l_ck_log_print(lua_State* L) {
	// Safely extract a string we got from Lua
	// example: fallout2.log.print(123), LuaJIT makes it '123'
	const char* message = luaL_checkstring(L, 1);

	if (message != nullptr) {
		// try send message to monitor
		fallout::displayMonitorAddMessage(message);
	}

	return 0; // nothing to return
}


int l_ck_spawn_critter(lua_State* L) {
    // argument #1 from Lua
    int pid = luaL_checkinteger(L, 1);

    fallout::Object* critter;

    if (fallout::objectCreateWithPid(&critter, pid) == 0) {
        fallout::objectAttemptPlacement(
            critter,
            fallout::gDude->tile,
            fallout::gDude->elevation, // originally fallout::gElevation
            3
        );
    }

	lua_pushboolean(L, true);
    return 1;
}

// ck scripting reload mods
void ck_reload_mods() {
    if (gLuaState == nullptr) {
		std::cout << "[CK] Cannot reload mods: Lua state is null" << std::endl;
        return;
    }

    lua_getglobal(gLuaState, "ckReloadMods");

    if (!lua_isfunction(gLuaState, -1)) {
		std::cout << "[CK] ckReloadMods() is not defined" << std::endl;
        lua_pop(gLuaState, 1);
        return;
    }

    if (lua_pcall(gLuaState, 0, 0, 0) != LUA_OK) {
		std::cout << "[CK] Reload Error: " << lua_tostring(gLuaState, -1) << std::endl;

        lua_pop(gLuaState, 1);
		return;
    }

	fallout::tileWindowRefresh();
}

// Init
void ck_scripting_init() {
    std::cout << "[CK] Initializing LuaJIT backend..." << std::endl;

	gLuaState = luaL_newstate();
	if (gLuaState != nullptr) {
		// Init global lua state
		luaL_openlibs(gLuaState);

		// create global ck module
		lua_newtable(gLuaState);               // stack: [ck]
		lua_setglobal(gLuaState, "ck");        // stack: []

		ck_requiref(gLuaState, "ck.rendering", luaopen_ck_rendering, 1);
		lua_pop(gLuaState, 1);

		ck_requiref(gLuaState, "ck.game_time", luaopen_ck_game_time, 1);
		lua_pop(gLuaState, 1);

		ck_requiref(gLuaState, "ck.map", luaopen_ck_map, 1);
		lua_pop(gLuaState, 1);

        // expand path to include fallout2-ck/ck/fallout2
        // Tells lua to search .lua files in ck/ (which is fallout2-ce/../ck)
        // luaL_dostring(gLuaState, "package.path = package.path .. ';../ck/?.lua'");
        // luaL_dostring(gLuaState, "package.path = package.path .. ';../ck/?/init.lua'");
		luaL_dostring(gLuaState, "package.path = package.path .. ';../?.lua;../?/init.lua'");

        // bindings. registers c <-> lua functions
		lua_register(gLuaState, "ckLogPrint", l_ck_log_print);
		lua_register(gLuaState, "ckSpawnCritter", l_ck_spawn_critter);

        // bootstrap
        int status = luaL_dofile(gLuaState, "../ck/fallout2/bootstrap.lua");
        if (status != 0) {
            std::cerr << "[CK] Lua Error: " << lua_tostring(gLuaState, -1) << std::endl;
        }
    } else {
        std::cerr << "[CK] Failed to initialize LuaJIT state!" << std::endl;
	}

	ck_assets_register_mod(gAssetRegistry, "temple_of_trials", "../mods/temple_of_trials/assets");

	CkFrm* frm = ck_assets_resolve(gAssetRegistry, "temple_of_trials:scenery/tree10");
	if (frm) {
		std::cout << "[CK TEST] resolved! " << frm->frames[0][0].width << "x" << frm->frames[0][0].height << std::endl;
	}

	// should be cached and not shown
	CkFrm* frm2 = ck_assets_resolve(gAssetRegistry, "temple_of_trials:scenery/tree10");
}

// Exit
void ck_scripting_exit() {
    if (gLuaState != nullptr) {
        std::cout << "[CK] Shutting down LuaJIT backend..." << std::endl;
        lua_close(gLuaState);
        gLuaState = nullptr;
    }
}

// this is called from fallout2-ce once interface is ready
void ck_scripting_on_game_start() {
	ck_call_hook("ckOnGameStart");
}

void ck_scripting_on_engine_ready() {
    std::cout << "[CK] Engine ready, initializing proto cache..." << std::endl;
    // gProtoCache.initialize("build/proto_cache.json");
}

void ck_scripting_on_game_loaded() {
	ck_call_hook("ckOnGameLoaded");
}

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
        std::cout << "[CK] Received new value for key " << key << ": " << result << "! " << std::endl;
    }

    // clear out the stack
    lua_pop(gLuaState, 1);

    return result;
}
