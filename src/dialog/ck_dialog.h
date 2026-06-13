#ifndef CK_DIALOG_H
#define CK_DIALOG_H

#include "obj_types.h"

namespace fallout {
    struct Object;
    struct Program; // forward declaration
}

namespace ck {

const int LUA_SCRIPT_SID_START = 50000;

extern int gLastDialogChoice;

void dialog_register_critter(fallout::Object* obj, int lua_script_id);
bool dialog_is_lua_critter(fallout::Object* obj);

void dialog_init();
bool dialog_try_handle(fallout::Object* speaker);

fallout::Program* dialog_get_dummy_program();

void dialog_set_reply(const char* text);
void dialog_add_option(const char* text);
int  dialog_go();
void dialog_exit();

}

#endif // CK_DIALOG_H
