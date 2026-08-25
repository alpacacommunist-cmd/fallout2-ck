#include "object/critter/ck_skills.h"
#include "ck_dispatcher/ck_dispatcher.h"

#include "skill_defs.h"
#include "random.h"
#include "proto.h"


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
	bool critter_skill_is_valid(int skill) { return skill >= 0 && skill < fallout::SKILL_COUNT; }

	int critter_set_skill(fallout::Object* critter, int skill, int value) {
		if (critter == nullptr) return -5;
		if (!critter_skill_is_valid(skill)) return -5;

		fallout::Proto* proto;
		if (fallout::protoGetProto(critter->pid, &proto) != 0 || proto == nullptr) return -5;

        if (value > 300) value = 300;
        if (value < 0)   value = 0;

        proto->critter.data.skills[skill] = value;

		return 0;
	}

	int critter_add_skill(fallout::Object* critter, int skill, int value) {
		fallout::Proto* proto;
		if (fallout::protoGetProto(critter->pid, &proto) != 0 || proto == nullptr) return -5;

		int new_base = proto->critter.data.skills[skill] + value;

		return critter_set_skill(critter, static_cast<fallout::Skill>(skill), new_base);
	}

    int critter_get_skill(fallout::Object* critter, int skill) {
		return fallout::skillGetValue(critter, static_cast<fallout::Skill>(skill));
    }

	int player_skill(int skill) {
		return critter_get_skill(fallout::gDude, skill);
	}

	int player_set_skill(int skill, int value) {
		return critter_set_skill(fallout::gDude, static_cast<fallout::Skill>(skill), value);
	}

	int player_add_skill(int skill, int value) {
		return critter_add_skill(fallout::gDude, static_cast<fallout::Skill>(skill), value);
	}
}

namespace ck::skills {
	static int g_outdoorsman_steps_accumulator = 0;

	void on_use_complete(fallout::Object* obj, int skill, fallout::Object* target, int success_count, int bonus) {
		if (obj != fallout::gDude) return;

        ck::dispatcher::on_skill_used(skill, success_count, bonus);
	}

	void on_attack_complete(fallout::Object* attacker, int skill, fallout::Object* target, int roll_result, int accuracy) {
        if (attacker != fallout::gDude) return;

		bool is_success  = (roll_result == fallout::ROLL_SUCCESS || roll_result == fallout::ROLL_CRITICAL_SUCCESS);
		int combat_bonus = accuracy - 90;

		on_use_complete(attacker, skill, target, is_success, combat_bonus);

		if ((fallout::dudeIsSneaking() || fallout::dudeHasState(fallout::DudeState(0))))
			on_use_complete(fallout::gDude, fallout::SKILL_SNEAK, target, is_success, (combat_bonus/2));
    }

    void on_encounter(int difficulty_modifier, int frequency, bool special) {
		int outdoorsman_bonus = difficulty_modifier - ((100 - frequency) / 2);
		if (special) outdoorsman_bonus -= 50;

		on_use_complete(fallout::gDude, fallout::SKILL_OUTDOORSMAN, nullptr, 1, outdoorsman_bonus);
	}

	void on_worldmap_step(int terrain_difficulty) {
        g_outdoorsman_steps_accumulator += terrain_difficulty;

        if (g_outdoorsman_steps_accumulator >= 150) {
            g_outdoorsman_steps_accumulator = 0;

            on_use_complete(fallout::gDude, fallout::SKILL_OUTDOORSMAN, nullptr, 1, -terrain_difficulty);
        }
    }
}

void ck_get_skills_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::SKILL_COUNT; ++i) callback(g_skill_names[i], i);
}

void ck_get_rolls_metadata(void (*callback)(const char* lua_name, int value)) {
	for (int i = 0; i < 4; ++i) callback(g_roll_names[i], i);
}

int player_skill(int skill) { return ck::player_skill(skill); }
int player_add_skill(int skill, int value) { return ck::player_add_skill(skill, value); }
int player_set_skill(int skill, int value) { return ck::player_set_skill(skill, value); }

int critter_get_skill(fallout::Object* critter, int skill) {
    return ck::critter_get_skill(critter, skill);
}

int critter_set_skill(fallout::Object* critter, int skill, int value) {
    return ck::critter_set_skill(critter, skill, value);
}

