#include "utils/fallout/types/script.h"
#include "fallout2-ce/scripts.h"

#include "log.h"
static constexpr logging::Context logger{"utils/fallout/types/script.cc"};

namespace ck::utils::fallout::types::script {
    // PROCS
    static_assert(static_cast<int>(Proc::NO_PROC) == ::fallout::SCRIPT_PROC_NO_PROC);
    static_assert(static_cast<int>(Proc::START) == ::fallout::SCRIPT_PROC_START);
    static_assert(static_cast<int>(Proc::SPATIAL) == ::fallout::SCRIPT_PROC_SPATIAL);
    static_assert(static_cast<int>(Proc::DESCRIPTION) == ::fallout::SCRIPT_PROC_DESCRIPTION);
    static_assert(static_cast<int>(Proc::PICKUP) == ::fallout::SCRIPT_PROC_PICKUP);
    static_assert(static_cast<int>(Proc::DROP) == ::fallout::SCRIPT_PROC_DROP);
    static_assert(static_cast<int>(Proc::USE) == ::fallout::SCRIPT_PROC_USE);
    static_assert(static_cast<int>(Proc::USE_OBJ_ON) == ::fallout::SCRIPT_PROC_USE_OBJ_ON);
    static_assert(static_cast<int>(Proc::USE_SKILL_ON) == ::fallout::SCRIPT_PROC_USE_SKILL_ON);
    static_assert(static_cast<int>(Proc::TALK) == ::fallout::SCRIPT_PROC_TALK);
    static_assert(static_cast<int>(Proc::CRITTER) == ::fallout::SCRIPT_PROC_CRITTER);
    static_assert(static_cast<int>(Proc::COMBAT) == ::fallout::SCRIPT_PROC_COMBAT);
    static_assert(static_cast<int>(Proc::DAMAGE) == ::fallout::SCRIPT_PROC_DAMAGE);
    static_assert(static_cast<int>(Proc::MAP_ENTER) == ::fallout::SCRIPT_PROC_MAP_ENTER);
    static_assert(static_cast<int>(Proc::MAP_EXIT) == ::fallout::SCRIPT_PROC_MAP_EXIT);
    static_assert(static_cast<int>(Proc::CREATE) == ::fallout::SCRIPT_PROC_CREATE);
    static_assert(static_cast<int>(Proc::DESTROY) == ::fallout::SCRIPT_PROC_DESTROY);
    static_assert(static_cast<int>(Proc::LOOK_AT) == ::fallout::SCRIPT_PROC_LOOK_AT);
    static_assert(static_cast<int>(Proc::TIMED) == ::fallout::SCRIPT_PROC_TIMED);
    static_assert(static_cast<int>(Proc::MAP_UPDATE) == ::fallout::SCRIPT_PROC_MAP_UPDATE);
    static_assert(static_cast<int>(Proc::PUSH) == ::fallout::SCRIPT_PROC_PUSH);
    static_assert(static_cast<int>(Proc::IS_DROPPING) == ::fallout::SCRIPT_PROC_IS_DROPPING);
    static_assert(static_cast<int>(Proc::COMBAT_IS_STARTING) == ::fallout::SCRIPT_PROC_COMBAT_IS_STARTING);
    static_assert(static_cast<int>(Proc::COMBAT_IS_OVER) == ::fallout::SCRIPT_PROC_COMBAT_IS_OVER);
    static_assert(static_cast<int>(Proc::COUNT) == ::fallout::SCRIPT_PROC_COUNT, "Proc count mismatch!");

    Proc proc(int proc) {
        return static_cast<Proc>(proc);
    }

    ::fallout::Script* get_script(int sid) {
        ::fallout::Script* script;
        int result = ::fallout::scriptGetScript(sid, &script);

        return (result != -1) ? script : nullptr;
    }

    int remove_script(int sid) {
        return ::fallout::scriptRemove(sid);
    }

    int exec_proc(int sid, Proc proc) {
        return ::fallout::scriptExecProc(sid, static_cast<::fallout::ScriptProc>(proc));
    }

    int proc(::fallout::Script* script, int proc) {
        return script->procs[proc];
    }

    int index(::fallout::Script* script) {
        return script->index;
    }

    int param(::fallout::Script* script) {
        return script->fixedParam;
    }

    ::fallout::Object* owner(::fallout::Script* script) {
        return script->owner;
    }

    void set_proc(::fallout::Script* script, Proc proc, int value) {
        script->procs[static_cast<int>(proc)] = value;
    }

    void set_local_vars_count(::fallout::Script* script, int value) {
        script->localVarsCount = value;
    }

    void set_overrides(::fallout::Script* script, int value) {
        script->scriptOverrides = value;
    }

    void set_index(::fallout::Script* script, int value) {
        script->index = value;
    }

    void set_no_save(int sid) {
        ::fallout::Script* script;
        ::fallout::scriptGetScript(sid, &script);

        if (script != nullptr) script->flags |= ::fallout::SCRIPT_FLAG_NO_SAVE;
    }
}
