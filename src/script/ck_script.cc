#include "ck_ids.h"
#include "ck_utils.h"
#include "ck_encoding.h"

#include "scripts.h"
#include "game_dialog.h"

#include "script/ck_script.h"
#include "object/ck_object_registry.h"

#include <cstring>

#include "ck_log.h"
static const Logger log("CK Script");

namespace ck {
	int gLastDialogChoice = -1;

	static fallout::Program gDummyProgram;
	static fallout::Script gDummyScript;

	static std::unordered_map<int, fallout::Script> g_lua_scripts;

	void reset_dummy_script() {
		log.debug("RESET gDummyScript");
		g_lua_scripts.clear();
	}

    fallout::Script* script_get_dummy(int sid) {
		int lua_id = lua_id_from_sid(clean_sid(sid));

        auto [it, inserted] = g_lua_scripts.try_emplace(lua_id);
        auto& script = it->second;

        if (inserted) {
            script.sp.built_tile = -1;
            script.sp.radius = -1;
            script.index = -1;
            script.localVarsOffset = -1;
            script.actionBeingUsed = -1;

            for (int i = 0; i < 25; i++) script.procs[i] = -1;
        }

        script.sid = sid;
        script.program = nullptr;

        return &script;
    }

	fallout::Program* program_get_dummy() {
		return &gDummyProgram;
	}

	bool owns_sid(int sid) {
		return ck::is_ck_sid(ck::clean_sid(sid));
	}

	void on_map_update(unsigned int ticks) {
		ck_dispatcher_on_map_update(ticks);
	}

	bool script_try_handle(int sid, int proc) {
		if (!is_ck_sid(clean_sid(sid))) return false;

		int lua_id = lua_id_from_sid(clean_sid(sid));
		const CkManagedObject* managed = gObjectRegistry.get_managed(lua_id);

		if (!managed) return false;

		int fixed_param = gDummyScript.fixedParam;
		gDummyScript.scriptOverrides = 0;

		bool handled_in_lua = ck_dispatcher_on_proc(lua_id, proc, fixed_param, managed->meta.mod_id.c_str());

		if (handled_in_lua) {
			gDummyScript.scriptOverrides = 1;
			return true;
		}

		log.warn("unhanled proc: {} for id: {}", proc, lua_id);
		return false;
	}

	int dialog_init_ui() {
		return fallout::_gdialogInitFromScript(-1, 0);
	}

	void dialog_set_reply(const char* text) {
		std::string converted = utf8_to_cp1251(text);
		fallout::gameDialogSetTextReply(program_get_dummy(), -4, converted.c_str());
	}

	void dialog_add_option(const char* text, int reaction) {
		std::string converted = utf8_to_cp1251(text);
		// proc=0 — no int procedures
		fallout::gameDialogAddTextOptionWithProc(-4, converted.c_str(), 0, reaction);
	}

	int dialog_go() {
		fallout::_gdialogGo();
		return gLastDialogChoice;
	}

	void dialog_exit() { fallout::gameDialogExit(); }
	void dialog_close_ui() { fallout::_gdialogExitFromScript(); }

} // namespace ck

// ffi
bool ck_dialog_init_ui() { return ck::dialog_init_ui() != -1; }
void ck_dialog_set_reply(const char* text) { ck::dialog_set_reply(text); }
void ck_dialog_add_option(const char* text, int reaction) { ck::dialog_add_option(text, reaction); }
int ck_dialog_go() { return ck::dialog_go(); }
void ck_dialog_exit() { ck::dialog_exit(); }
void ck_dialog_close_ui() { ck::dialog_close_ui(); }
