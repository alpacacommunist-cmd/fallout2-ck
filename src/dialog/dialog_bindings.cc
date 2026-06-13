#include "dialog/ck_dialog.h"
#include "game_dialog.h"

extern "C" {
#include "../../src/vendor/luajit/src/lauxlib.h"
}

namespace {
    // ck.dialog.set_reply(text)
    static int l_set_reply(lua_State* L) {
        const char* text = luaL_checkstring(L, 1);
        fallout::gameDialogSetTextReply(ck::dialog_get_dummy_program(), -4, text);
        return 0;
    }

    // ck.dialog.add_option(text, proc_id)
    static int l_add_option(lua_State* L) {
        const char* text = luaL_checkstring(L, 1);
        int proc_id = luaL_checkinteger(L, 2);
        fallout::gameDialogAddTextOptionWithProc(-4, text, proc_id, 0);
        return 0;
    }

    // proc_id = ck.dialog.go()
    static int l_go(lua_State* L) {
        int rc = fallout::_gdialogGo();
        lua_pushinteger(L, rc);
        return 1;
    }

    // ck.dialog.barter(modifier)
    static int l_barter(lua_State* L) {
        int modifier = luaL_optinteger(L, 1, 0);
        int result = fallout::gameDialogBarter(modifier);
        fallout::gameDialogEndBarter();
        lua_pushinteger(L, result);
        return 1;
    }

    static const luaL_Reg dialog_lib[] = {
        { "set_reply", l_set_reply },
        { "add_option", l_add_option },
        { "go", l_go },
        { "barter", l_barter },
        { nullptr, nullptr }
    };
}

int luaopen_ck_dialog(lua_State* L) {
    luaL_newlib(L, dialog_lib);
    return 1;
}
