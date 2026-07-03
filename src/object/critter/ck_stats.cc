#include "object/critter/ck_stats.h"

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
	int critter_base_stat(fallout::Object* critter, int stat) {
		return fallout::critterGetStat(critter, stat);
	}

	int critter_set_base_stat(fallout::Object* critter, int stat, int value) {
		return fallout::critterSetBaseStat(critter, stat, value);
	}

	int critter_bonus_stat(fallout::Object* critter, int stat) {
		return fallout::critterGetBonusStat(critter, stat);
	}

	int critter_set_bonus_stat(fallout::Object* critter, int stat, int value) {
		return fallout::critterSetBonusStat(critter, stat, value);
	}

	int critter_pc_stat(int stat) { return fallout::pcGetStat(stat); }
	int critter_get_hp(fallout::Object* critter) { return fallout::critterGetHitPoints(critter); }

	int critter_get_max_hp(fallout::Object* critter) {
		return critter_base_stat(critter, fallout::STAT_MAXIMUM_HIT_POINTS);
	}

	int critter_adjust_hp(fallout::Object* critter, int target_hp) {
		int current_hp = critter_get_hp(critter);
		int delta      = target_hp - current_hp;

		return fallout::critterAdjustHitPoints(critter, delta);
	}

	int critter_set_full_hp(fallout::Object* critter) {
		return fallout::critterAdjustHitPoints(critter, critter_get_max_hp(critter));
	}
}

void ck_get_stats_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::STAT_COUNT; ++i) callback(g_stat_names[i], i);
}

void ck_get_pc_stats_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::PC_STAT_COUNT; ++i) callback(g_pc_stat_names[i], i);
}

int ck_critter_get_base_stat(void* ptr, int stat_id) {
    if (!ptr) return -1; auto* critter = static_cast<fallout::Object*>(ptr);

	return ck::critter_base_stat(critter, stat_id);
}

bool ck_critter_set_base_stat(void* ptr, int stat, int value) {
	if (!ptr) return false; auto* critter = static_cast<fallout::Object*>(ptr);

	return (ck::critter_set_base_stat(critter, stat, value) == 0);
}

int ck_critter_get_bonus_stat(void* ptr, int stat_id) {
    if (!ptr) return -1; auto* critter = static_cast<fallout::Object*>(ptr);

	return ck::critter_bonus_stat(critter, stat_id);
}

bool ck_critter_set_bonus_stat(void* ptr, int stat, int value) {
	if (!ptr) return false; auto* critter = static_cast<fallout::Object*>(ptr);

	return (ck::critter_set_bonus_stat(critter, stat, value) == 0);
}

int player_stat(int stat) { return ck::critter_base_stat(fallout::gDude, stat); }
int player_pc_stat(int stat) { return ck::critter_pc_stat(stat); }


int ck_critter_get_hp(void* ptr) {
	if (!ptr) return false; auto* critter = static_cast<fallout::Object*>(ptr);

	return ck::critter_get_hp(critter);
}

int ck_critter_get_max_hp(void* ptr) {
	if (!ptr) return false; auto* critter = static_cast<fallout::Object*>(ptr);

	return ck::critter_get_max_hp(critter);
}

void ck_critter_set_current_hp(void* ptr, int target_hp) {
    if (!ptr) return; auto* critter = static_cast<fallout::Object*>(ptr);

	ck::critter_adjust_hp(critter, target_hp);
}

int ck_critter_set_full_hp(void* ptr) {
    if (!ptr) return -1; auto* critter = static_cast<fallout::Object*>(ptr);

	return ck::critter_set_full_hp(critter);
}

