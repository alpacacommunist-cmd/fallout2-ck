#ifndef CK_DIALOG_H
#define CK_DIALOG_H

#include "obj_types.h"

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
extern "C" {
    void ck_dialog_set_reply(const char* text);
    void ck_dialog_add_option(const char* text, int reaction);
    int  ck_dialog_go();
    void ck_dialog_exit();
}

#endif // CK_DIALOG_H
