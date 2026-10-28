#ifndef CK_SCRIPT_H
#define CK_SCRIPT_H

#include "ck_api.h"

namespace fallout {
	struct Object;
	struct Script;
    struct Program;
}

namespace ck::script {
    void set_last_dialog_choice(int option_index);
    bool script_proc_enabled(fallout::Script* script, int proc);

    void reset_vars(fallout::Script* script);
    void set_no_save(int sid);

    void kick_off_map_updates_for_sid(int sid);
    void disable_map_updates_for_object(fallout::Object* object);
    void enable_map_updates_for_object(fallout::Object* object);

    void remove_object_script(fallout::Object* object);
    void assign_proto_script(fallout::Object* object);
    void assign_script_index_to_object(int script_index, fallout::Object* object);

	bool owns_sid(int sid);
	void on_map_update(unsigned int ticks);
    void handle_global_proc_event(int sid, int proc);
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
