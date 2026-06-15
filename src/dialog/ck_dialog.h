#ifndef CK_DIALOG_H
#define CK_DIALOG_H

#include "ck_api.h"

namespace fallout {
    struct Object;
    struct Program; // forward declaration
}

namespace ck {
	typedef enum GameDialogReaction {
		GAME_DIALOG_REACTION_GOOD = 49,
		GAME_DIALOG_REACTION_NEUTRAL = 50,
		GAME_DIALOG_REACTION_BAD = 51,
	} GameDialogReaction;

	extern int gLastDialogChoice;

	void dialog_init();
	bool dialog_try_handle(fallout::Object* speaker);

	fallout::Program* dialog_get_dummy_program();
}

// ffi

CK_API void ck_dialog_set_reply(const char* text);
CK_API void ck_dialog_add_option(const char* text, int reaction);
CK_API int  ck_dialog_go();
CK_API void ck_dialog_exit();

#endif // CK_DIALOG_H
