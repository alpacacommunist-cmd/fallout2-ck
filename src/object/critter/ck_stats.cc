#include "object/critter/ck_stats.h"
#include "stat_defs.h"

#include <stdexcept>

#include "ck_log.h"
static const Logger log("CK Stats");

static const char* g_pc_stat_names[] = {
    "unspent_skills", "level", "experience", "reputation", "karma"
};

static_assert(sizeof(g_pc_stat_names) / sizeof(g_pc_stat_names[0]) == fallout::PC_STAT_COUNT,
              "pc stat names / stat_defs.h mismatch!");

static const char* g_stat_names[] = {
    "strength", "perception", "endurance", "charisma", "intelligence", "agility", "luck",
    "max_hp", "max_ap", "armor_class", "unarmed_damage", "melee_damage", "carry_weight",
    "sequence", "healing_rate", "critical_chance", "better_criticals",
    "dt_normal", "dt_laser", "dt_fire", "dt_plasma", "dt_electrical", "dt_emp", "dt_explosion",
    "dr_normal", "dr_laser", "dr_fire", "dr_plasma", "dr_electrical", "dr_emp", "dr_explosion",
    "radiation_resistance", "poison_resistance", "age", "gender",
    "hp", "current_poison_level", "current_radiation_level"
};

static_assert(sizeof(g_stat_names) / sizeof(g_stat_names[0]) == fallout::STAT_COUNT,
              "stat names / stat_defs.h mismatch!");


namespace ck {
	int critter_stat(fallout::Object* critter, int stat) {
		return fallout::critterGetStat(critter, stat);
	}

	int critter_pc_stat(int stat) {
		return fallout::pcGetStat(stat);
	}
}

int player_stat(int stat) {
	return ck::critter_stat(fallout::gDude, stat);
}

int player_pc_stat(int stat) {
	return ck::critter_pc_stat(stat);
}

void ck_get_stats_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::STAT_COUNT; ++i) callback(g_stat_names[i], i);
}

void ck_get_pc_stats_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::PC_STAT_COUNT; ++i) callback(g_pc_stat_names[i], i);
}
