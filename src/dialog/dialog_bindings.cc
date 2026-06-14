#include "dialog/ck_dialog.h"

extern "C" {
#include "../../src/vendor/luajit/src/lauxlib.h"
}

namespace {
    static int l_set_reply(lua_State* L) {
        const char* text = luaL_checkstring(L, 1);
        ck::dialog_set_reply(text);
        return 0;
    }

    static int l_add_option(lua_State* L) {
        const char* text = luaL_checkstring(L, 1);
		int reaction     = luaL_optinteger(L, 2, ck::GAME_DIALOG_REACTION_NEUTRAL);
        ck::dialog_add_option(text, reaction);
        return 0;
    }

    static int l_go(lua_State* L) {
        int choice = ck::dialog_go();
        lua_pushinteger(L, choice);
        return 1;
    }

    static int l_exit(lua_State* L) {
        ck::dialog_exit();
        return 0;
    }

    static const luaL_Reg dialog_lib[] = {
        { "set_reply",  l_set_reply },
        { "add_option", l_add_option },
        { "go",         l_go },
        { "exit",       l_exit },
        { nullptr,      nullptr }
    };
}

int luaopen_ck_dialog(lua_State* L) {
    luaL_newlib(L, dialog_lib);
    return 1;
}
