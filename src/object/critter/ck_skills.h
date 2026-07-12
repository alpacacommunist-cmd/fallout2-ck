#ifndef CK_SKILLS_H
#define CK_SKILLS_H

#include "ck_api.h"

extern "C" const char* ck_get_current_mod_id();

namespace fallout {
    struct Object;
	extern Object* gDude;
	struct Attack;

	int skillGetValue(Object* critter, int skill);
}

namespace ck {
	int player_skill(int skill);
}

namespace ck::skills {
	void on_use_complete(fallout::Object* obj, int skill, fallout::Object* target, int success_count, int bonus);
}

CK_API void ck_get_skills_metadata(void (*callback)(const char* lua_name, int value));
CK_API void ck_get_rolls_metadata(void (*callback)(const char* lua_name, int value));

CK_API int player_skill(int skill);

#endif
