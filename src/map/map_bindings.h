// src/map/map_bindings.h
#ifndef CK_SCRIPTING_MAP_BINDINGS_H
#define CK_SCRIPTING_MAP_BINDINGS_H

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}

int luaopen_ck_map(lua_State* L);

#endif
