// src/game_time/game_time_bindings.cc
#include "game_time/game_time_bindings.h"
#include "scripts.h"

extern "C" {
#include "../../src/vendor/luajit/src/lauxlib.h"
}

namespace {

	//
	// void gameTimeGetDate(int* monthPtr, int* dayPtr, int* yearPtr)
	// 
    // l_ck_get_year -> ck.game_time.get_year
    int l_ck_get_year(lua_State* L) {
        int year = 0;
        fallout::gameTimeGetDate(nullptr, nullptr, &year);
        lua_pushinteger(L, year);
        return 1;
    }

    int l_ck_get_day(lua_State* L) {
        int day = 0;
        fallout::gameTimeGetDate(nullptr, &day, nullptr);
        lua_pushinteger(L, day);
        return 1;
    }

    int l_ck_get_month(lua_State* L) {
        int month = 0;
        fallout::gameTimeGetDate(&month, nullptr, nullptr);
        lua_pushinteger(L, month);
        return 1;
    }

    int l_ck_get_hour(lua_State* L) {
        lua_pushinteger(L, fallout::gameTimeGetHour());
        return 1;
    }

    int l_ck_get_total_days(lua_State* L) {
        unsigned int gameTime = fallout::gameTimeGetTime();
        int totalDays = gameTime / (10 * 60 * 60 * 24);
        lua_pushinteger(L, totalDays);
        return 1;
    }

    const luaL_Reg game_time_lib[] = {
        { "get_year",       l_ck_get_year },
        { "get_day",        l_ck_get_day },
        { "get_month",      l_ck_get_month },
        { "get_hour",       l_ck_get_hour },
        { "get_total_days", l_ck_get_total_days },
        { nullptr,          nullptr }
    };
}

int luaopen_ck_game_time(lua_State* L) {
    luaL_newlib(L, game_time_lib);
    return 1;
}
