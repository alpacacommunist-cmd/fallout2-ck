#ifndef CK_SKILLS_H
#define CK_SKILLS_H

#include "ck_api.h"

namespace fallout {
    struct Object;
}

namespace ck {
}

CK_API void ck_get_skills_metadata(void (*callback)(const char* lua_name, int value));

#endif
