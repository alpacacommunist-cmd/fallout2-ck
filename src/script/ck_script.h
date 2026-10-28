#ifndef CK_SCRIPT_H
#define CK_SCRIPT_H

#include "ck_api.h"

namespace fallout {
	struct Object;
	struct Script;
    struct Program;
}

namespace ck::script {
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

    void set_last_dialog_choice(int option_index);
    bool proc_enabled(fallout::Script* script, int proc);
    void disable_proc(fallout::Script* script, int proc);
    void enable_proc(fallout::Script* script, int proc);

    void reset_vars(fallout::Script* script);
    void set_no_save(int sid);
    int index(fallout::Script* script);
    int param(fallout::Script* script);
    fallout::Object* owner(fallout::Script* script);
    fallout::Object* source(fallout::Script* script);
    void overrides(fallout::Script* script, int value);

    void kick_off_map_updates_for_sid(int sid);
    void disable_map_updates_for_object(fallout::Object* object);
    void enable_map_updates_for_object(fallout::Object* object);

    void remove_object_script(fallout::Object* object);
    void assign_proto_script(fallout::Object* object);
    void assign_script_index_to_object(int script_index, fallout::Object* object);
}

// ffi

CK_API bool ck_dialog_init_ui();
CK_API void ck_dialog_set_reply(const char* text);
CK_API void ck_dialog_add_option(const char* text, int reaction);
CK_API int  ck_dialog_go();
CK_API void ck_dialog_exit();
CK_API void ck_dialog_close_ui();
CK_API void ck_script_disable_map_updates_for_object(fallout::Object* object);

#endif // CK_DIALOG_H
