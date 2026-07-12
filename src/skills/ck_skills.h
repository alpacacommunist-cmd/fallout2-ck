#ifndef CK_SKILLS_H
#define CK_SKILLS_H

#include "ck_api.h"

namespace fallout {
    struct Object;
	extern Object* gDude;

	int skillGetValue(Object* critter, int skill);
}

namespace ck {
	int player_skill(int skill);
}

CK_API void ck_get_skills_metadata(void (*callback)(const char* lua_name, int value));

CK_API int player_skill(int skill);

#endif
