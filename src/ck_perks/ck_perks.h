#ifndef CK_PERKS_H
#define CK_PERKS_H

#include "ck_api.h"

namespace fallout {
    struct Object;
	extern Object* gDude;

    enum Perk : int;
}

namespace ck::perks {
}

CK_API void ck_get_perks_metadata(void (*callback)(const char* name, int value));

#endif
