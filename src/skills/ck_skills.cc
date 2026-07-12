#include "skills/ck_skills.h"
#include "skill_defs.h"
#include "ck_dispatcher/ck_dispatcher.h"

static const char* g_skill_names[] = {
    "small_guns", "big_guns", "energy_weapons", "unarmed", "melee_weapons", "throwing",
    "first_aid", "doctor", "sneak", "lockpick", "steal", "traps", "science", "repair",
    "speech", "barter", "gambling", "outdoorsman"
};

static_assert(sizeof(g_skill_names) / sizeof(g_skill_names[0]) == fallout::SKILL_COUNT,
              "skill names / skill_defs.h mismatch!");

void ck_get_skills_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::SKILL_COUNT; ++i) callback(g_skill_names[i], i);
}
