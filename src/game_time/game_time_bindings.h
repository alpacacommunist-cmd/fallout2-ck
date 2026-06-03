// src/game_time/game_time_bindings.h
#ifndef CK_SCRIPTING_GAME_TIME_BINDINGS_H
#define CK_SCRIPTING_GAME_TIME_BINDINGS_H

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}

int luaopen_ck_game_time(lua_State* L);

#endif
