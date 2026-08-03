#include "ck_ids.h"
#include "ck_utils.h"
#include "ck_encoding.h"

#include "scripts.h"
#include "game_dialog.h"

#include "script/ck_script.h"
#include "ck_registry/ck_registry.h"

#include <cstring>

#include "ck_log.h"
static const Logger logger("CK Script");

namespace ck {
	int gLastDialogChoice = -1;

	static fallout::Program gDummyProgram;
	static fallout::Script gDummyScript;

	static std::unordered_map<int, fallout::Script> g_lua_scripts;

	void reset_dummy_script() {
		logger.debug("RESET lua_scripts");
		g_lua_scripts.clear();
	}

    fallout::Script* script_get_dummy(int sid) {
		int lua_id = ck::ids::lua_id_from_sid(sid);

        auto [it, inserted] = g_lua_scripts.try_emplace(lua_id);
        auto& script = it->second;

        if (inserted) {
            script.sp.built_tile = -1;
            script.sp.radius = -1;
            script.index = -1;
            script.localVarsOffset = -1;
            script.actionBeingUsed = -1;

			script.index = 0;
			script.flags |= (SCRIPT_FLAG_LOADED | SCRIPT_FLAG_EXECUTED);

            for (int i = 0; i < fallout::SCRIPT_PROC_COUNT; i++) script.procs[i] = -1;
        }

        script.sid = sid;
        script.program = nullptr;

        return &script;
    }

	fallout::Program* program_get_dummy() {
		return &gDummyProgram;
	}

	bool owns_sid(int sid) {
		return ck::ids::is_ck_sid(sid);
	}

	void on_map_update(unsigned int ticks) {
		ck_dispatcher_on_map_update(ticks);
	}

	bool script_try_handle(int sid, int proc) {
		if (!ck::ids::is_ck_sid(sid)) return false;

		int lua_id = ck::ids::lua_id_from_sid(sid);
		const LuaMeta* meta = ck::registry::get_meta(lua_id);

		if (meta == nullptr) return false; // game loading

		if (meta->mod_id.empty()) {
			logger.warn("Object with LuaID {} found in SIDs, but missing in registries", lua_id);
			return false;
		}

		fallout::Script* script = script_get_dummy(sid);

		int fixed_param = script->fixedParam;
		script->scriptOverrides = 0;

		// void* source_ptr = script->source;

		bool handled_in_lua = ck_dispatcher_on_proc(lua_id, proc, fixed_param, meta->mod_id.c_str());

		if (handled_in_lua) {
			script->scriptOverrides = 1;
			return true;
		}

		logger.warn("unhandled proc: {} for id: {}", proc, lua_id);
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
