#ifndef CK_ASSETS_BINDINGS_H
#define CK_ASSETS_BINDINGS_H

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}

int luaopen_ck_assets(lua_State* L);

#endif
