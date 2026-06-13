#include "dialog/ck_dialog.h"

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
	static std::unordered_map<fallout::Object*, int> gLuaCritters;
	void dialog_register_critter(fallout::Object* obj, int lua_script_id) {
		if (obj != nullptr) gLuaCritters[obj] = lua_script_id;
	}
	bool dialog_is_lua_critter(fallout::Object* obj) {
		if (obj == nullptr) return false;

		return gLuaCritters.find(obj) != gLuaCritters.end();
	}

	// not in header
	typedef enum GameDialogReaction {
		GAME_DIALOG_REACTION_GOOD = 49,
		GAME_DIALOG_REACTION_NEUTRAL = 50,
		GAME_DIALOG_REACTION_BAD = 51,
	} GameDialogReaction;

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
		std::cout << "[CK] speaker sid: " << speaker->sid << std::endl;

		auto it = ck::gLuaCritters.find(speaker);
		std::cout << "[CK] found lua critter: " << speaker->sid << std::endl;
		if (it == ck::gLuaCritters.end()) return false; // skip to engine

		int lua_script_id = it->second;
		std::cout << "[CK] speaker lua id: " << lua_script_id << std::endl;

		if (fallout::ckOpenDialogUI(speaker) == -1) {
			std::cerr << "[CK Dialog Error]: Failed to initialize Dialogue UI!" << std::endl;
			return false;
		}

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

		fallout::ckCloseDialogUI();

		return true;
	}

	void dialog_set_reply(const char* text) {
		fallout::gameDialogSetTextReply(dialog_get_dummy_program(), -4, text);
	}

	void dialog_add_option(const char* text) {
		// proc=0 — no int procedures
		fallout::gameDialogAddTextOptionWithProc(-4, text, 0, GAME_DIALOG_REACTION_NEUTRAL);
	}

	int dialog_go() {
		fallout::_gdialogGo();
		return gLastDialogChoice;
	}

	void dialog_exit() {
		fallout::gameDialogExit();
	}

} // namespace ck
