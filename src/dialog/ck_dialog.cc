#include "dialog/ck_dialog.h"
#include "object/ck_object_registry.h"

#include "interpreter.h" // Program struct
#include "game_dialog.h"

#include <iostream>
#include <algorithm>
#include <unordered_map>

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
#include "../../src/vendor/luajit/src/lauxlib.h"
}

extern lua_State* gLuaState;

namespace ck {
	static fallout::Program gDummyProgram;
	int gLastDialogChoice = -1;

	void dialog_init() {
		std::fill(reinterpret_cast<char*>(&gDummyProgram),
				reinterpret_cast<char*>(&gDummyProgram) + sizeof(fallout::Program), 0);
	}

	fallout::Program* dialog_get_dummy_program() {
		return &gDummyProgram;
	}

	bool dialog_try_handle(fallout::Object* speaker) {
		if (speaker == nullptr || gLuaState == nullptr) return false;

		int lua_id = gObjectRegistry.find_by_ptr(speaker);
		if (lua_id == -1) return false;

		if (fallout::ckOpenDialogUI(speaker) == -1) {
			std::cerr << "[CK Dialog Error]: Failed to initialize Dialogue UI!" << std::endl;
			return false;
		}

		lua_getglobal(gLuaState, "ckOnDialogStart");
		if (lua_isfunction(gLuaState, -1)) {
			lua_pushinteger(gLuaState, lua_id);

			if (lua_pcall(gLuaState, 1, 0, 0) != 0) {
				std::cerr << "[CK Dialog Error]: " << lua_tostring(gLuaState, -1) << std::endl;
				lua_pop(gLuaState, 1);
			}
		} else {
			lua_pop(gLuaState, 1);
			std::cerr << "[CK Dialog Warning]: ckOnDialogStart is not defined in Lua!" << std::endl;
		}

		fallout::ckCloseDialogUI();

		return true;
	}

	void dialog_set_reply(const char* text) {
		fallout::gameDialogSetTextReply(dialog_get_dummy_program(), -4, text);
	}

	void dialog_add_option(const char* text, int reaction) {
		// proc=0 — no int procedures
		fallout::gameDialogAddTextOptionWithProc(-4, text, 0, reaction);
	}

	int dialog_go() {
		fallout::_gdialogGo();
		return gLastDialogChoice;
	}

	void dialog_exit() {
		fallout::gameDialogExit();
	}

} // namespace ck
