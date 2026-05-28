#include <iostream>

#include "ck_scripting.h"
#include "ck_rendering.h"

// bindings (requirements)
#include "display_monitor.h"
#include "proto_instance.h"
#include "map.h"
#include "scripts.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
#include "../../src/vendor/luajit/src/lualib.h"
#include "../../src/vendor/luajit/src/lauxlib.h"
}

// lua state global pointer, lives as long as game lives
lua_State* gLuaState = nullptr;


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

// l_ck_get_year -> ckGetYear -> fallout2.game_time.getYear
int l_ck_get_year(lua_State* L) {
	int year = 0;
	fallout::gameTimeGetDate(nullptr, nullptr, &year);

	lua_pushinteger(L, year);
    return 1; // one return value for lua
}

// l_ck_get_day -> ckGetDay -> fallout2.game_time.getDay
int l_ck_get_day(lua_State* L) {
    int day = 0;

    fallout::gameTimeGetDate(nullptr, &day, nullptr);

    lua_pushinteger(L, day);
    return 1; // one return value for lua
}

// l_ck_get_month -> ckGetMonth -> fallout2.game_time.getMonth
int l_ck_get_month(lua_State* L) {
    int month = 0;

    fallout::gameTimeGetDate(&month, nullptr, nullptr);

    lua_pushinteger(L, month);
    return 1; // one return value for lua
}

// l_ck_get_hour -> ckGetHour -> fallout2.game_time.getHour
int l_ck_get_hour(lua_State* L) {
    int hour = fallout::gameTimeGetHour();

    lua_pushinteger(L, hour);
    return 1; // one return value for lua
}

// l_ck_get_total_days -> ckGetTotalDays -> fallout2.game_time.getTotalDays
int l_ck_get_total_days(lua_State* L) {
    unsigned int gameTime = fallout::gameTimeGetTime();
    // 10 ticks = 1 second
    // 60 sec * 60 min * 24 hours = seconds/day
    int totalDays = gameTime / (10 * 60 * 60 * 24);

    lua_pushinteger(L, totalDays);

    return 1; // one value returned to Lua
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
    int offsetX = luaL_checkinteger(L, 3);
    int offsetY = luaL_checkinteger(L, 4);

    ck_rendering_add_scenery(fid, tile, offsetX, offsetY);
    return 0;
}

static const luaL_Reg rendering_lib[] = {
    { "draw_scenery", l_draw_scenery },
	{ "add_scenery", l_add_scenery },
    { nullptr, nullptr }
};

static int luaopen_ck_rendering(lua_State* L) {
    luaL_newlib(L, rendering_lib);

    return 1;
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

		// create rendering module
		lua_pushcfunction(gLuaState, luaopen_ck_rendering); // stack: [ck, rendering_module]
		lua_call(gLuaState, 0, 1);

		// assign rendering_module into ck.rendering
		lua_getglobal(gLuaState, "ck");           // stack: [rendering_module, ck]
		lua_pushvalue(gLuaState, -2);             // stack: [rendering_module, ck, rendering_module]
		lua_setfield(gLuaState, -2, "rendering"); // ck.rendering = rendering_module

		// cleanup stack: pop rendering_module and ck
		lua_pop(gLuaState, 2);


        // expand path to include fallout2-ck/ck/fallout2
        // Tells lua to search .lua files in ck/ (which is fallout2-ce/../ck)
        luaL_dostring(gLuaState, "package.path = package.path .. ';../ck/?.lua'");

        // bindings. registers c <-> lua functions
		lua_register(gLuaState, "ckLogPrint", l_ck_log_print);
		lua_register(gLuaState, "ckGetYear", l_ck_get_year);
		lua_register(gLuaState, "ckGetDay", l_ck_get_day);
		lua_register(gLuaState, "ckGetMonth", l_ck_get_month);
		lua_register(gLuaState, "ckGetHour", l_ck_get_hour);
		lua_register(gLuaState, "ckGetTotalDays", l_ck_get_total_days);
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
