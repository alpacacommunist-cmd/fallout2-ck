#include <iostream>

#include "ck_scripting.h"
#include "game_time/ck_game_time.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}
extern lua_State* gLuaState;

void ck_scripting_on_day_passed() {
	ck_call_hook("ckOnDayPassed");
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

