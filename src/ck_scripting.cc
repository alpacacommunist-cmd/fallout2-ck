#include <iostream>
#include <cstring>

#include "ck_scripting.h"
#include "ck_rendering.h"
#include "game_time/game_time_bindings.h"
#include "ck_debug_overlay/ck_debug_overlay.h"

// bindings (requirements)
#include "display_monitor.h"
#include "proto_instance.h"
#include "map.h"
#include "tile.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
#include "../../src/vendor/luajit/src/lualib.h"
#include "../../src/vendor/luajit/src/lauxlib.h"
}

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

//
// void gameTimeGetDate(int* monthPtr, int* dayPtr, int* yearPtr)
// 

// l_ck_get_map_id -> ckGetMapId -> fallout2.map.getMapID
int l_ck_get_map_id(lua_State* L) {
    lua_pushinteger(L, fallout::mapGetCurrentMap());
    return 1;
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

// rendering
static int l_draw_scenery(lua_State* L) {
    int fid = luaL_checkinteger(L, 1);
    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);

    ck_rendering_draw_scenery(fid, x, y);
    return 0;
}

static int l_add_scenery(lua_State* L) {
    int fid = luaL_checkinteger(L, 1);
    int tile = luaL_checkinteger(L, 2);

    ck_rendering_add_scenery(fid, tile);
    return 0;
}

static int l_add_tile(lua_State* L) {
    int fid = luaL_checkinteger(L, 1);
    int tile = luaL_checkinteger(L, 2);

    ck_rendering_add_tile(fid, tile);
    return 0;
}

static int l_set_camera_borders(lua_State* L) {
    int left = luaL_checkinteger(L, 1);
    int right = luaL_checkinteger(L, 2);
    int top = luaL_checkinteger(L, 3);
    int bottom = luaL_checkinteger(L, 4);

	ck_rendering_set_camera_borders(left, right, top, bottom);
	return 0;
}

static int l_clear_rendering(lua_State* L) {
    ck_rendering_clear();
    return 0;
}

static const luaL_Reg rendering_lib[] = {
    { "draw_scenery", l_draw_scenery },
	{ "add_scenery", l_add_scenery },
	{ "add_tile", l_add_tile },
	{ "clear", l_clear_rendering },
	{ "set_camera_borders", l_set_camera_borders },
    { nullptr, nullptr }
};

static int luaopen_ck_rendering(lua_State* L) {
    luaL_newlib(L, rendering_lib);

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
//
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

        // expand path to include fallout2-ck/ck/fallout2
        // Tells lua to search .lua files in ck/ (which is fallout2-ce/../ck)
        luaL_dostring(gLuaState, "package.path = package.path .. ';../ck/?.lua'");

        // bindings. registers c <-> lua functions
		lua_register(gLuaState, "ckLogPrint", l_ck_log_print);
		lua_register(gLuaState, "ckGetMapId", l_ck_get_map_id);

		lua_register(gLuaState, "ckSpawnCritter", l_ck_spawn_critter);

        // bootstrap
        int status = luaL_dofile(gLuaState, "../ck/fallout2/bootstrap.lua");
        if (status != 0) {
            std::cerr << "[CK] Lua Error: " << lua_tostring(gLuaState, -1) << std::endl;
        }
    } else {
        std::cerr << "[CK] Failed to initialize LuaJIT state!" << std::endl;
    }
}

// Exit
//
void ck_scripting_exit() {
    if (gLuaState != nullptr) {
        std::cout << "[CK] Shutting down LuaJIT backend..." << std::endl;
        lua_close(gLuaState);
        gLuaState = nullptr;
    }
}

// this is called from fallout2-ce once interface is ready
// work in progress chill
void ck_scripting_on_game_start() {
    if (gLuaState == nullptr) return;

    // search global lua table for function "ckOnGameStart"
    // and put it on top of lua stack
    lua_getglobal(gLuaState, "ckOnGameStart");

    // check if it's indeed a function
    if (lua_isfunction(gLuaState, -1)) {
        // run it!
        // params lua_pcall: state, nargs (0), nresults (0), msgh (0)
        int status = lua_pcall(gLuaState, 0, 0, 0);
        
        if (status != LUA_OK) {
            std::cerr << "[CK] Hook Error (onGameStart): " << lua_tostring(gLuaState, -1) << std::endl;
            lua_pop(gLuaState, 1); // clears error out of stack
        }
    } else {
        // no such function, remove it from stack
        lua_pop(gLuaState, 1);
    }
}

void ck_scripting_on_day_passed() {
    if (gLuaState == nullptr) return;

    // search global lua table for function "ckOnDayPassed"
    // and put it on top of lua stack
    lua_getglobal(gLuaState, "ckOnDayPassed");

    if (lua_isfunction(gLuaState, -1)) {
        // run it!
        // params lua_pcall: state, nargs (0), nresults (0), msgh (0)
		int status = lua_pcall(gLuaState, 0, 0, 0);

        if (status != LUA_OK) {
            std::cerr << "[CK] Hook Error " << "(onDayPassed): " << lua_tostring(gLuaState, -1) << std::endl;
            lua_pop(gLuaState, 1); // clears error out of stack
        }
    } else {
        // no such function, remove it from stack
        lua_pop(gLuaState, 1);
    }
}

// just in case
void ck_scripting_on_after_rest(int hours, int minutes) {
	ck_scripting_on_time_advance(hours, minutes);
}

void ck_scripting_on_time_advance(int hours, int minutes) {
    if (gLuaState == nullptr) return;

    lua_getglobal(gLuaState, "ckOnTimeAdvance");

    if (lua_isfunction(gLuaState, -1)) {
        lua_pushinteger(gLuaState, hours);
        lua_pushinteger(gLuaState, minutes);

        int status = lua_pcall(gLuaState, 2, 0, 0);

        if (status != LUA_OK) {
            std::cerr << "[CK] Hook Error " << "(onTimeAdvance): " << lua_tostring(gLuaState, -1) << std::endl;
            lua_pop(gLuaState, 1); // clears error out of stack
        }
    } else {
        // no such function, remove it from stack
        lua_pop(gLuaState, 1);
    }
}

void ck_scripting_on_game_loaded() {
    if (gLuaState == nullptr) return;

    // search global lua table for function "ckOnGameLoaded"
    // and put it on top of lua stack
    lua_getglobal(gLuaState, "ckOnGameLoaded");

    if (lua_isfunction(gLuaState, -1)) {
        // run it!
        // params lua_pcall: state, nargs (0), nresults (0), msgh (0)
		int status = lua_pcall(gLuaState, 0, 0, 0);

        if (status != LUA_OK) {
            std::cerr << "[CK] Hook Error " << "(onGameLoaded): " << lua_tostring(gLuaState, -1) << std::endl;
            lua_pop(gLuaState, 1); // clears error out of stack
        }
    } else {
        // no such function, remove it from stack
        lua_pop(gLuaState, 1);
    }
}

void ck_scripting_on_map_enter() {
    if (gLuaState == nullptr) return;

	ck_rendering_clear();
	if (ck_debug_overlay_enabled()) ck_debug_overlay_toggle();

    // search global lua table for function "ckOnMapEnter"
    lua_getglobal(gLuaState, "ckOnMapEnter");

    if (lua_isfunction(gLuaState, -1)) {
        int status = lua_pcall(gLuaState, 0, 0, 0);

        if (status != LUA_OK) {
            std::cerr << "[CK] Hook Error " << "(onMapEnter): " << lua_tostring(gLuaState, -1) << std::endl;
            lua_pop(gLuaState, 1);
        }
    } else {
        lua_pop(gLuaState, 1);
    }
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
