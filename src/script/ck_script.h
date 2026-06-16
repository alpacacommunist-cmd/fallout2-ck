#ifndef CK_SCRIPT_H
#define CK_SCRIPT_H

#include "ck_api.h"

namespace fallout {
    struct Object;
	struct Script;
}

namespace ck {
	bool owns_sid(int sid);
	fallout::Script* script_get_dummy(int sid);

	bool script_try_handle(int sid, int proc);
}

// ffi

#endif // CK_DIALOG_H
