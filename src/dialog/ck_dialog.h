#ifndef CK_DIALOG_H
#define CK_DIALOG_H

#include "obj_types.h"

namespace fallout {
    struct Program; // forward declaration
}

namespace ck {

const int LUA_SCRIPT_SID_START = 50000;

void dialog_init();

bool dialog_try_handle(fallout::Object* speaker);

fallout::Program* dialog_get_dummy_program();

}

#endif // CK_DIALOG_H
