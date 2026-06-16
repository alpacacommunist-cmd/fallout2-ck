#include "ck_ids.h"
#include "ck_utils.h"

#include "scripts.h"
#include "game_dialog.h"
#include "display_monitor.h"

#include "script/ck_script.h"
#include "object/ck_object_registry.h"

#include <iostream>
#include <string>

namespace ck {
	int gLastDialogChoice = -1;

	static fallout::Program gDummyProgram;
	static fallout::Script gDummyScript;

	bool owns_sid(int sid) {
		return ck::is_ck_sid(ck::clean_sid(sid));
	}

	fallout::Script* script_get_dummy(int sid) {
		gDummyScript.sid = sid;
		return &gDummyScript;
	}

	fallout::Program* program_get_dummy() {
		return &gDummyProgram;
	}

	bool script_try_handle(int sid, int proc) {
		if (!is_ck_sid(clean_sid(sid))) return false;

		int lua_id = lua_id_from_sid(clean_sid(sid));
		const CkManagedObject* managed = gObjectRegistry.get_managed(lua_id);

		if (!managed) return false;

		gDummyScript.scriptOverrides = 0;

		switch (proc) {
			case fallout::SCRIPT_PROC_LOOK_AT:
				if (!managed->meta.name.empty())
					fallout::displayMonitorAddMessage(managed->meta.name.c_str());

				gDummyScript.scriptOverrides = 1;
				return true;

			case fallout::SCRIPT_PROC_DESCRIPTION:
				if (!managed->meta.description.empty()) {
					fallout::displayMonitorAddMessage(managed->meta.description.c_str());
				}

				gDummyScript.scriptOverrides = 1;
				return true;

			case fallout::SCRIPT_PROC_TALK:
				if (fallout::_gdialogInitFromScript(-1, 0) == -1) {
					std::cerr << "[CK Dialog Error]: Failed to initialize Dialogue UI!" << std::endl;
					return false;
				}

				ck_call_lua_hook("ckOnDialogStart", lua_id);

				fallout::_gdialogExitFromScript();

				return true;

			default:
				if (managed->meta.proto_sid != -1 && managed->ptr) {
					int saved         = managed->ptr->sid;
					managed->ptr->sid = managed->meta.proto_sid;
					fallout::scriptExecProc(managed->meta.proto_sid, proc);
					managed->ptr->sid = saved;

					return true;
				}

				return false;
		}
	}

	void dialog_set_reply(const char* text) {
		fallout::gameDialogSetTextReply(program_get_dummy(), -4, text);
	}

	void dialog_add_option(const char* text, int reaction) {
		// proc=0 — no int procedures
		fallout::gameDialogAddTextOptionWithProc(-4, text, 0, reaction);
	}

	int dialog_go() {
		fallout::_gdialogGo();
		return gLastDialogChoice;
	}

	void dialog_exit() { fallout::gameDialogExit(); }

} // namespace ck

// ffi
extern "C" {
void ck_dialog_set_reply(const char* text) { ck::dialog_set_reply(text); }
void ck_dialog_add_option(const char* text, int reaction) { ck::dialog_add_option(text, reaction); }
int ck_dialog_go() { return ck::dialog_go(); }
void ck_dialog_exit() { ck::dialog_exit(); }
}
