// src/dialog/dialog_bindings.h
#ifndef CK_SCRIPTING_DIALOG_BINDINGS_H
#define CK_SCRIPTING_DIALOG_BINDINGS_H

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
}

int luaopen_ck_dialog(lua_State* L);

#endif
