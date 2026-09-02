#ifndef CK_SCRIPT_H
#define CK_SCRIPT_H

#include "ck_api.h"

struct CkObjectFFI;

namespace fallout {
	struct Script;
    struct Program;
}

namespace ck::script {
	void reset();

	extern int gLastDialogChoice;

	typedef enum GameDialogReaction {
		GAME_DIALOG_REACTION_GOOD = 49,
		GAME_DIALOG_REACTION_NEUTRAL = 50,
		GAME_DIALOG_REACTION_BAD = 51,
	} GameDialogReaction;

	fallout::Script*  get_dummy(int sid);
	fallout::Program* program_get_dummy();

    void assign_no_save_to_sid(int sid);
    void kick_off_map_updates_for_sid(int sid);
    void disable_map_updates_for_sid(int sid);
    void enable_map_updates_for_sid(int sid);

	bool owns_sid(int sid);
	void on_map_update(unsigned int ticks);
    void handle_global_proc_event(int sid, int proc);
	bool try_handle(int sid, int proc);
}

// ffi

CK_API bool ck_dialog_init_ui();
CK_API void ck_dialog_set_reply(const char* text);
CK_API void ck_dialog_add_option(const char* text, int reaction);
CK_API int  ck_dialog_go();
CK_API void ck_dialog_exit();
CK_API void ck_dialog_close_ui();


#endif // CK_DIALOG_H
