#include "ck_ids.h"
#include "ck_encoding.h"

#include "scripts.h"
#include "game_dialog.h"
#include "obj_types.h"

#include "script/ck_script.h"
#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "ck_proto/registry/ck_proto_registry.h"
#include "ck_dispatcher/ck_dispatcher.h"

#include "ck_log.h"
static const Logger logger("CK Script");

namespace fallout {
    struct Object;
	struct Script;
    struct Program;

	void displayMonitorAddMessage(const char* str);
}

namespace ck::common {
    const char* system_mod_id();
    const char* current_mod_id();
}

namespace ck::script {
	int gLastDialogChoice = -1;

	static fallout::Program gDummyProgram;
	static fallout::Script gDummyScript;

	static std::unordered_map<int, fallout::Script> g_lua_scripts;

	void reset() {
		logger.debug("RESET lua_scripts");
		g_lua_scripts.clear();
	}

    fallout::Script* get_dummy(int sid) {
		int lua_id = ck::ids::lua_id_from_sid(sid);

        auto [it, inserted] = g_lua_scripts.try_emplace(lua_id);
        auto& script = it->second;

        if (inserted) {
            script.sp.built_tile = -1;
            script.sp.radius = -1;
            script.index = -1;
            script.localVarsOffset = -1;
            script.actionBeingUsed = -1;

            script.index = -1;
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
        ck::dispatcher::on_map_update(ticks);
	}

    bool proto_handle(int sid, int proc) {
        int pid = ck::proto::get_pid_by_sid(sid);

		fallout::Script* script = get_dummy(sid);

		int fixed_param = script->fixedParam;
		script->scriptOverrides = 0;

        std::string mod_id = ck::common::system_mod_id();
		bool handled_in_lua = ck::dispatcher::on_proto_proc(pid, proc, fixed_param, mod_id.c_str());

		if (handled_in_lua) {
			script->scriptOverrides = 1;
			return true;
		}

		logger.warn("[proto] unhandled proc: {} for sid: {}", proc, sid);
		return false;
    }

	bool try_handle(int sid, int proc) {
		if (!ck::ids::is_ck_sid(sid)) return false;

        if (ck::ids::is_ck_proto_sid(sid)) return proto_handle(sid, proc);

		int lua_id = ck::ids::lua_id_from_sid(sid);
		const LuaMeta* meta = ck::registry::get_meta(lua_id);

		if (meta == nullptr) return false; // game loading
        // logger.warn("object {} pid {} sid {}", meta->tag, meta->source_pid, ck::ids::clean_sid(sid));

		if (meta->mod_id.empty()) {
			logger.warn("Object with LuaID {} found in SIDs, but missing in registries", lua_id);
			return false;
		}

		fallout::Script* script = get_dummy(sid);

		int fixed_param = script->fixedParam;
		script->scriptOverrides = 0;

		// void* source_ptr = script->source;

		bool handled_in_lua = ck::dispatcher::on_proc(lua_id, proc, fixed_param, meta->mod_id.c_str());

		if (handled_in_lua) {
			script->scriptOverrides = 1;
			return true;
		}

		logger.warn("unhandled proc: {} for id: {}", proc, lua_id);
		return false;
	}

    void handle_global_proc_event(int sid, int proc) {
        if (proc != 18) return; // 18 = SCRIPT_PROC_DESTROY

        fallout::Script* script = nullptr;
        if (fallout::scriptGetScript(sid, &script) == -1 || script == nullptr) return;

        fallout::Object* victim_ptr = script->owner;
        fallout::Object* killer_ptr = script->source;

        if (victim_ptr == nullptr || fallout::objectTypeFromPid(victim_ptr->pid) != fallout::OBJ_TYPE_CRITTER) return;

        CkObjectFFI victim{}, killer{};
        if (victim_ptr) ck::object::to_ffi(victim, victim_ptr);
        if (killer_ptr) ck::object::to_ffi(killer, killer_ptr);

        if (!victim_ptr || !killer_ptr) return;

        ck::dispatcher::on_critter_killed(&victim, &killer);

        // object was spawned by lua mod but has no lua script
        if (victim.lua_id != -1 && !ck::ids::is_ck_sid(victim_ptr->sid)) {
            const LuaMeta* meta = ck::registry::get_meta(victim.lua_id);

            // dispatch proc manually
            logger.debug("Dispatching proc {} to lua_id {} manually (no lua script found)", proc, victim.lua_id);
            ck::dispatcher::on_proc(victim.lua_id, proc, 0, meta->mod_id.data());
        }
    }

    void set_no_save(int sid) {
        fallout::Script* script;
        fallout::scriptGetScript(sid, &script);

        if (script != nullptr) script->flags |= SCRIPT_FLAG_NO_SAVE;
    }

    void kick_off_map_updates_for_sid(int sid) {
        if (sid == -1) return;

        fallout::scriptExecProc(sid, fallout::SCRIPT_PROC_START);
        fallout::scriptExecProc(sid, fallout::SCRIPT_PROC_MAP_ENTER);
    }

    void disable_map_updates_for_object(fallout::Object* object) {
        fallout::Script* script;
        fallout::scriptGetScript(object->sid, &script);

        if (script != nullptr) {
            script->procs[fallout::SCRIPT_PROC_TIMED]   = fallout::SCRIPT_PROC_NO_PROC;
            script->procs[fallout::SCRIPT_PROC_CRITTER] = fallout::SCRIPT_PROC_NO_PROC;
            script->procs[fallout::SCRIPT_PROC_MAP_UPDATE] = fallout::SCRIPT_PROC_NO_PROC;
        }
    }

    void assign_script_index_to_object(int script_index, fallout::Object* object) {
        if (object->sid != -1) fallout::scriptRemove(object->sid);

        fallout::scriptAdd(&object->sid, ck::ids::script_type_for_object(object));

        fallout::Script* script;
        fallout::scriptGetScript(object->sid, &script);

        if (script != nullptr) {
            object->scriptIndex = script_index;
            script->ownerId = object->id;
            script->owner   = object;

            script->index = script_index;
            script->flags |= SCRIPT_FLAG_NO_SAVE;
        }
    }

    void enable_map_updates_for_object(fallout::Object* object) {
        fallout::Script* script;
        fallout::scriptGetScript(object->sid, &script);
        if (script != nullptr) {
            logger.debug("enabling map_update for SID: {}", script->sid);

            script->procs[fallout::SCRIPT_PROC_TIMED]   = 1;
            script->procs[fallout::SCRIPT_PROC_CRITTER] = 1;
            script->procs[fallout::SCRIPT_PROC_MAP_UPDATE] = 1;
        }
    }

	int dialog_init_ui() {
		return fallout::_gdialogInitFromScript(-1, fallout::HeadFidget(0));
	}

	void dialog_set_reply(const char* text) {
		std::string converted = utf8_to_cp1251(std::string_view(text));
		fallout::gameDialogSetTextReply(program_get_dummy(), -4, converted.c_str());
	}

	void dialog_add_option(const char* text, int reaction) {
		std::string converted = utf8_to_cp1251(std::string_view(text));
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
bool ck_dialog_init_ui() { return ck::script::dialog_init_ui() != -1; }
void ck_dialog_set_reply(const char* text) { ck::script::dialog_set_reply(text); }
void ck_dialog_add_option(const char* text, int reaction) { ck::script::dialog_add_option(text, reaction); }
int ck_dialog_go() { return ck::script::dialog_go(); }
void ck_dialog_exit() { ck::script::dialog_exit(); }
void ck_dialog_close_ui() { ck::script::dialog_close_ui(); }
