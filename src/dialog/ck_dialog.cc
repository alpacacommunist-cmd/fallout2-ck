#include "ck_ids.h"
#include "ck_utils.h"
#include "dialog/ck_dialog.h"
#include "object/ck_object_registry.h"

#include "game_dialog.h"

#include <iostream>
#include <algorithm>

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

		std::cout << "[CK Dialog]: Speaker sid: " << ck::clean_sid(speaker->sid) << std::endl;

		if (fallout::ckOpenDialogUI(speaker) == -1) {
			std::cerr << "[CK Dialog Error]: Failed to initialize Dialogue UI!" << std::endl;
			return false;
		}

		ck_call_lua_hook("ckOnDialogStart", lua_id);

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


extern "C" {
	void ck_dialog_set_reply(const char* text) {
		ck::dialog_set_reply(text);
	}

	void ck_dialog_add_option(const char* text, int reaction) {
		ck::dialog_add_option(text, reaction);
	}

	int ck_dialog_go() {
		return ck::dialog_go();
	}

	void ck_dialog_exit() {
		ck::dialog_exit();
	}
}
