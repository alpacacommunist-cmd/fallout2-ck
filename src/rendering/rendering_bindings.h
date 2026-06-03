// src/game_time/game_time_bindings.h
#ifndef CK_SCRIPTING_RENDERING_BINDINGS_H
#define CK_SCRIPTING_RENDERING_BINDINGS_H

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}

int luaopen_ck_rendering(lua_State* L);

#endif
