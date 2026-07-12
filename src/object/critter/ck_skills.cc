#include "object/critter/ck_skills.h"
#include "ck_dispatcher/ck_dispatcher.h"

#include "skill_defs.h"
#include "random.h"
#include "item.h"


#include "ck_log.h"
static const Logger log("CK Skills");

static const char* g_skill_names[] = {
    "small_guns", "big_guns", "energy_weapons", "unarmed", "melee_weapons", "throwing",
    "first_aid", "doctor", "sneak", "lockpick", "steal", "traps", "science", "repair",
    "speech", "barter", "gambling", "outdoorsman"
};

static const char* g_roll_names[] = {
    "crit_failure", "failure", "success", "crit_success"
};

static_assert(sizeof(g_skill_names) / sizeof(g_skill_names[0]) == fallout::SKILL_COUNT,
              "skill names / skill_defs.h mismatch!");

static_assert(sizeof(g_roll_names) / sizeof(g_roll_names[0]) == 4,
              "roll names / random.h mismatch!");

namespace ck {
	int player_skill(int skill) {
		return fallout::skillGetValue(fallout::gDude, skill);
	}
}

namespace ck::skills {
	void on_use_complete(fallout::Object* obj, int skill, fallout::Object* target, int success_count, int bonus) {
		if (obj != fallout::gDude) return;

		ck_dispatcher_on_skill_used(skill, success_count, bonus);
	}

	void on_attack_complete(fallout::Object* attacker, int skill, fallout::Object* target, int roll_result, int accuracy) {
        if (attacker != fallout::gDude) return;

		bool is_success  = (roll_result == fallout::ROLL_SUCCESS || roll_result == fallout::ROLL_CRITICAL_SUCCESS);
		int combat_bonus = accuracy - 90;

		on_use_complete(attacker, skill, target, is_success, combat_bonus);
    }
}

void ck_get_skills_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::SKILL_COUNT; ++i) callback(g_skill_names[i], i);
}

CK_API void ck_get_rolls_metadata(void (*callback)(const char* lua_name, int value)) {
	for (int i = 0; i < 4; ++i) callback(g_roll_names[i], i);
}

int player_skill(int skill) { return ck::player_skill(skill); }
