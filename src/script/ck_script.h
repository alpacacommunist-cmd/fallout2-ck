#ifndef CK_SCRIPT_H
#define CK_SCRIPT_H

#include "ck_api.h"

namespace fallout {
    struct Object;
	struct Script;
    struct Program;
}

namespace ck {
	typedef enum GameDialogReaction {
		GAME_DIALOG_REACTION_GOOD = 49,
		GAME_DIALOG_REACTION_NEUTRAL = 50,
		GAME_DIALOG_REACTION_BAD = 51,
	} GameDialogReaction;

	extern int gLastDialogChoice;

	bool owns_sid(int sid);
	fallout::Script*  script_get_dummy(int sid);
	fallout::Program* program_get_dummy();

	bool script_try_handle(int sid, int proc);
}

// ffi

CK_API bool ck_dialog_init_ui();
CK_API void ck_dialog_set_reply(const char* text);
CK_API void ck_dialog_add_option(const char* text, int reaction);
CK_API int  ck_dialog_go();
CK_API void ck_dialog_exit();
CK_API void ck_dialog_close_ui();


#endif // CK_DIALOG_H
