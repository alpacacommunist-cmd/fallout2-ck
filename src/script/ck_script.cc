#include "ck_ids.h"
#include "system/encoding.h"
#include "utils.h"

#include "fallout2-ce/game_dialog.h"
#include "fallout2-ce/scripts.h"

#include "ck_dispatcher/ck_dispatcher.h"
#include "ck_proto/registry/ck_proto_registry.h"
#include "ck_registry/ck_registry.h"
#include "object/ck_object.h"
#include "script/ck_script.h"
#include "ck_mods/ck_mods.h"

#include "log.h"
static constexpr logging::Context logger{"ck_script.cc"};

namespace fallout {
    int objectSetScriptFromProto(Object* object, int* sidPtr);
}

namespace ck::script {
	static int last_dialog_choice = -1;

	static fallout::Program gDummyProgram;
    fallout::Program* program_get_dummy() {
		return &gDummyProgram;
	}

    void set_last_dialog_choice(int option_index) {
        last_dialog_choice = option_index;
    }

    bool script_proc_enabled(fallout::Script* script, int proc) {
        return script->procs[proc] != fallout::SCRIPT_PROC_NO_PROC;
    }

    void reset_vars(fallout::Script* script) {
        if (script) script->localVarsCount = 0;
    }

    void set_no_save(int sid) {
        fallout::Script* script;
        fallout::scriptGetScript(sid, &script);

        if (script != nullptr) script->flags |= fallout::SCRIPT_FLAG_NO_SAVE;
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

            logger.debug("Disabled PROC_CRITTER, PROC_MAP_UPDATE, PROC_TIMED for pid: {}", object->pid);
        }
    }

    void enable_map_updates_for_object(fallout::Object* object) {
        fallout::Script* script;
        fallout::scriptGetScript(object->sid, &script);
        if (script != nullptr) {
            script->procs[fallout::SCRIPT_PROC_TIMED]   = 1;
            script->procs[fallout::SCRIPT_PROC_CRITTER] = 1;
            script->procs[fallout::SCRIPT_PROC_MAP_UPDATE] = 1;

            logger.debug("enabling map_update for SID: {}", script->sid);
        }
    }

    // Removes both current script and scriptIndex
    void remove_object_script(fallout::Object* object) {
        if (object->sid != -1) fallout::scriptRemove(object->sid);
        object->scriptIndex = -1;
    }

    void assign_proto_script(fallout::Object* object) {
        ck::script::remove_object_script(object);

        fallout::objectSetScriptFromProto(object, &object->sid);
        ck::script::set_no_save(object->sid);
    }

    void assign_script_index_to_object(int script_index, fallout::Object* object) {
        ck::script::remove_object_script(object);

        fallout::scriptAdd(&object->sid, static_cast<fallout::ScriptType>(ck::ids::script_type_for_object(object)));

        fallout::Script* script;
        fallout::scriptGetScript(object->sid, &script);

        if (script != nullptr) {
            object->scriptIndex = script_index;
            script->ownerId = object->id;
            script->owner   = object;

            script->index = script_index;
            script->flags |= fallout::SCRIPT_FLAG_NO_SAVE;
        }
    }

    bool handle_proc(int sid, fallout::Script* script, int proc) {
        // skip if script isn't processed
        if (!ck::ids::is_ck_script_index(script->index)) return false;
        // skip map updates (if proc disabled)
        if (proc == fallout::SCRIPT_PROC_CRITTER && !script_proc_enabled(script, fallout::SCRIPT_PROC_CRITTER)) return false;

        bool handled = false;

        int lua_id = ck::registry::find_by_ptr(script->owner);
        if (lua_id != -1) {
            const LuaMeta* meta = ck::registry::get_meta(lua_id);
            handled = ck::dispatcher::on_proc(lua_id, proc, script->fixedParam, meta->mod_id);
        } else {
            int pid = ck::proto::registry::get_pid_by_sid(script->index);
            if (pid == -1) return false;

            handled = ck::dispatcher::on_proto_proc(pid, proc, script->fixedParam, ck::mods::system_mod_id());
        }

        if (handled) {
            script->scriptOverrides = 1;
            return true;
        } else {
            logger.warning("unhandled proc: {} (script_index: {})", proc, script->index);
        }

        return false;
    };

    void handle_global_proc_event(int sid, int proc) {
        if (proc != 18) return; // 18 = SCRIPT_PROC_DESTROY

        fallout::Script* script = nullptr;
        if (fallout::scriptGetScript(sid, &script) == -1 || script == nullptr) return;

        fallout::Object* victim_ptr = script->owner;
        fallout::Object* killer_ptr = script->source;

        if (victim_ptr == nullptr || fallout::ProtoId(victim_ptr->pid).objectType() != fallout::OBJ_TYPE_CRITTER) return;

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
            ck::dispatcher::on_proc(victim.lua_id, proc, 0, meta->mod_id);
        }
    }

	int dialog_init_ui() {
		return fallout::_gdialogInitFromScript(fallout::HeadFrmId(), fallout::HeadFidget(0));
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
		return last_dialog_choice;
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

void ck_script_disable_map_updates_for_object(fallout::Object* object) {
    CK_ENSURE_VALID_OBJECT(object);
    ck::script::disable_map_updates_for_object(object);
}
