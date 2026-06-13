#include "dialog/ck_dialog.h"
#include "interpreter.h" // Program struct
#include <iostream>

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
#include "../../src/vendor/luajit/src/lauxlib.h"
}

extern lua_State* gLuaState;

namespace ck {

static fallout::Program gDummyProgram;

void dialog_init() {
    std::fill(reinterpret_cast<char*>(&gDummyProgram),
              reinterpret_cast<char*>(&gDummyProgram) + sizeof(fallout::Program), 0);
}

fallout::Program* dialog_get_dummy_program() {
    return &gDummyProgram;
}

bool dialog_try_handle(fallout::Object* speaker) {
    if (speaker == nullptr || gLuaState == nullptr) return false;

    int sid = speaker->sid;
    if (sid < LUA_SCRIPT_SID_START) {
        return false; // regular npc, pass to engine
    }

    int lua_script_id = sid - LUA_SCRIPT_SID_START;

    lua_getglobal(gLuaState, "ckOnDialogStart");
    if (lua_isfunction(gLuaState, -1)) {
        lua_pushinteger(gLuaState, lua_script_id);
        lua_pushlightuserdata(gLuaState, speaker);

        if (lua_pcall(gLuaState, 2, 0, 0) != 0) {
            std::cerr << "[CK Dialog Error]: " << lua_tostring(gLuaState, -1) << std::endl;
            lua_pop(gLuaState, 1);
        }
    } else {
        lua_pop(gLuaState, 1);
		std::cerr << "[CK Dialog Warning]: ckOnDialogStart is not defined in Lua!" << std::endl;
	}

    return true;
}

} // namespace
