#ifndef CK_TYPES_SCRIPT_H
#define CK_TYPES_SCRIPT_H

namespace fallout {
    struct Script;
    struct Object;
}

namespace ck::utils::fallout::types::script {
    enum class Proc : int {
        NO_PROC = 0,
        START = 1,
        SPATIAL = 2,
        DESCRIPTION = 3,
        PICKUP = 4,
        DROP = 5,
        USE = 6,
        USE_OBJ_ON = 7,
        USE_SKILL_ON = 8,
        PROC_9 = 9,
        PROC_10 = 10,
        TALK = 11,
        CRITTER = 12,
        COMBAT = 13,
        DAMAGE = 14,
        MAP_ENTER = 15,
        MAP_EXIT = 16,
        CREATE = 17,
        DESTROY = 18,
        PROC_19 = 19,
        PROC_20 = 20,
        LOOK_AT = 21,
        TIMED = 22,
        MAP_UPDATE = 23,
        PUSH = 24,
        IS_DROPPING = 25,
        COMBAT_IS_STARTING = 26,
        COMBAT_IS_OVER = 27,
        COUNT
    };

    constexpr const char *proc_to_string(Proc proc) {
        switch (proc) {
        case Proc::NO_PROC:            return "NO_PROC";
        case Proc::START:              return "START";
        case Proc::SPATIAL:            return "SPATIAL";
        case Proc::DESCRIPTION:        return "DESCRIPTION";
        case Proc::PICKUP:             return "PICKUP";
        case Proc::DROP:               return "DROP";
        case Proc::USE:                return "USE";
        case Proc::USE_OBJ_ON:         return "USE_OBJ_ON";
        case Proc::USE_SKILL_ON:       return "USE_SKILL_ON";
        case Proc::PROC_9:             return "USE_AD_ON";
        case Proc::PROC_10:            return "USE_DISAD_ON";
        case Proc::TALK:               return "TALK";
        case Proc::CRITTER:            return "CRITTER";
        case Proc::COMBAT:             return "COMBAT";
        case Proc::DAMAGE:             return "DAMAGE";
        case Proc::MAP_ENTER:          return "MAP_ENTER";
        case Proc::MAP_EXIT:           return "MAP_EXIT";
        case Proc::CREATE:             return "CREATE";
        case Proc::DESTROY:            return "DESTROY";
        case Proc::PROC_19:            return "BARTER_INIT";
        case Proc::PROC_20:            return "BARTER";
        case Proc::LOOK_AT:            return "LOOK_AT";
        case Proc::TIMED:              return "TIMED";
        case Proc::MAP_UPDATE:         return "MAP_UPDATE";
        case Proc::PUSH:               return "PUSH";
        case Proc::IS_DROPPING:        return "IS_DROPPING";
        case Proc::COMBAT_IS_STARTING: return "COMBAT_IS_STARTING";
        case Proc::COMBAT_IS_OVER:     return "COMBAT_IS_OVER";
        default:                       return "UNKNOWN_PROC";
        }
    }

    constexpr const char *proc_to_string(int proc) {
        return proc_to_string(static_cast<Proc>(proc));
    }

    Proc proc(int proc);
    int exec_proc(int sid, Proc proc);

    ::fallout::Script* get_script(int sid);
    int remove_script(int sid);

    int proc(::fallout::Script* script, int proc);
    int index(::fallout::Script* script);
    int param(::fallout::Script* script);
    ::fallout::Object* owner(::fallout::Script* script);

    void set_proc(::fallout::Script* script, Proc proc, int value);
    void set_local_vars_count(::fallout::Script* script, int value);
    void set_overrides(::fallout::Script* script, int value);
    void set_index(::fallout::Script* script, int value);

    void set_no_save(int sid);
}

#endif
