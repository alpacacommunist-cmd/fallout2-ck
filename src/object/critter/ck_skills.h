#ifndef CK_SKILLS_H
#define CK_SKILLS_H

#include "ck_api.h"

extern "C" const char* ck_get_current_mod_id();

namespace fallout {
    struct Object;
	struct Attack;
	enum Skill : int;
    enum DudeState : int;

	extern Object* gDude;

	int skillGetValue(Object* critter, Skill skill);
	int critterGetStat(Object* critter, int stat);
	bool dudeIsSneaking();
    bool dudeHasState(DudeState state);
	bool skillIsTagged(int skill);
}

namespace ck {
	bool critter_skill_is_valid(int skill);
	int  critter_set_skill(fallout::Object* critter, int skill, int value);

	int player_skill(int skill);
	int player_skill_add(int skill, int value);
}

namespace ck::skills {
	void on_use_complete(fallout::Object* obj, int skill, fallout::Object* target, int success_count, int bonus);
	void on_attack_complete(fallout::Object* attacker, int skill, fallout::Object* target, int roll_result, int accuracy);
    void on_encounter(int difficulty_modifier, int frequency, bool special);
	void on_worldmap_step(int terrain_difficulty);
}

CK_API void ck_get_skills_metadata(void (*callback)(const char* lua_name, int value));
CK_API void ck_get_rolls_metadata(void (*callback)(const char* lua_name, int value));

CK_API int player_skill(int skill);
CK_API int player_add_skill(int skill, int value);
CK_API int player_set_skill(int skill, int value);
CK_API int ck_critter_get_skill(fallout::Object* critter, int skill);
CK_API int ck_critter_set_skill(fallout::Object* critter, int skill, int value);

#endif
