#include <iostream>

#include "ck_scripting.h"

// bindings (requirements)
#include "display_monitor.h"

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


// Init
//
void ckScriptingInit() {
    std::cout << "[CK] Initializing LuaJIT backend..." << std::endl;

    gLuaState = luaL_newstate();
    if (gLuaState != nullptr) {
        // Init global lua state
        luaL_openlibs(gLuaState);

        // expand path to include fallout2-ck/ck/fallout2
        // Tells lua to search .lua files in ck/ (which is fallout2-ce/../ck)
        luaL_dostring(gLuaState, "package.path = package.path .. ';../ck/?.lua'");

        // bindings. registers c <-> lua functions
        lua_register(gLuaState, "ckLogPrint", l_ck_log_print);

        // try execute sample lua script
        int status = luaL_dofile(gLuaState, "../mods/username/test.lua");
        if (status != 0) {
            std::cerr << "[CK] Lua Error: " << lua_tostring(gLuaState, -1) << std::endl;
        }
    } else {
        std::cerr << "[CK] Failed to initialize LuaJIT state!" << std::endl;
    }
}

// Exit
//
void ckScriptingExit() {
    if (gLuaState != nullptr) {
        std::cout << "[CK] Shutting down LuaJIT backend..." << std::endl;
        lua_close(gLuaState);
        gLuaState = nullptr;
    }
}


// this is called from fallout2-ce once interface is ready
// work in progress chill
void ckHookOnGameStart() {
    if (gLuaState == nullptr) return;

    // search global lua table for function "ckOnGameStart"
    // and put it on top of lua stack
    lua_getglobal(gLuaState, "ckOnGameStart");

    // check if it's indeed a function
    if (lua_isfunction(gLuaState, -1)) {
        // run it!
        // params lua_pcall: state, nargs (0), nresults (0), msgh (0)
        int status = lua_pcall(gLuaState, 0, 0, 0);
        
        if (status != 0) {
            std::cerr << "[CK] Hook Error (onGameStart): " << lua_tostring(gLuaState, -1) << std::endl;
            lua_pop(gLuaState, 1); // clears error out of stack
        }
    } else {
        // no such function, remove it from stack
        lua_pop(gLuaState, 1);
    }
}

int ckGetConfigInt(const char* key, int default_value) {
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
