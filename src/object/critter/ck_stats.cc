#include "object/critter/ck_stats.h"

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
		return fallout::critterGetStat(critter, static_cast<fallout::Stat>(stat));
	}

	int critter_set_base_stat(fallout::Object* critter, int stat, int value) {
		return fallout::critterSetBaseStat(critter, static_cast<fallout::Stat>(stat), value);
	}

	int critter_bonus_stat(fallout::Object* critter, int stat) {
		return fallout::critterGetBonusStat(critter, static_cast<fallout::Stat>(stat));
	}

	int critter_set_bonus_stat(fallout::Object* critter, int stat, int value) {
		return fallout::critterSetBonusStat(critter, static_cast<fallout::Stat>(stat), value);
	}

	int critter_pc_stat(int stat) { return fallout::pcGetStat(static_cast<fallout::PcStat>(stat)); }
	int critter_get_hp(fallout::Object* critter) { return fallout::critterGetHitPoints(critter); }

	int critter_get_max_hp(fallout::Object* critter) {
		return fallout::critterGetStat(critter, static_cast<fallout::Stat>(fallout::STAT_MAXIMUM_HIT_POINTS));
	}

	int critter_adjust_hp(fallout::Object* critter, int target_hp) {
		int current_hp = critter_get_hp(critter);
		int delta      = target_hp - current_hp;

		return fallout::critterAdjustHitPoints(critter, delta);
	}

	int critter_set_full_hp(fallout::Object* critter) {
		return ck::critter_adjust_hp(critter, critter_get_max_hp(critter));
	}
}

void ck_get_stats_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::STAT_COUNT; ++i) callback(g_stat_names[i], i);
}

void ck_get_pc_stats_metadata(void (*callback)(const char* lua_name, int value)) {
    for (int i = 0; i < fallout::PC_STAT_COUNT; ++i) callback(g_pc_stat_names[i], i);
}

template<typename R, typename F>
static R with_critter(void* ptr, R default_val, F&& func) {
    if (!ptr) return default_val;
    return func(static_cast<fallout::Object*>(ptr));
}

int ck_critter_get_base_stat(void* ptr, int stat_id) {
    return with_critter(ptr, -1, [=](auto* c) { return ck::critter_base_stat(c, stat_id); });
}

bool ck_critter_set_base_stat(void* ptr, int stat, int value) {
    return with_critter(ptr, false, [=](auto* c) { return ck::critter_set_base_stat(c, stat, value) == 0; });
}

int ck_critter_get_bonus_stat(void* ptr, int stat_id) {
    return with_critter(ptr, -1, [=](auto* c) { return ck::critter_bonus_stat(c, stat_id); });
}

bool ck_critter_set_bonus_stat(void* ptr, int stat, int value) {
    return with_critter(ptr, false, [=](auto* c) { return ck::critter_set_bonus_stat(c, stat, value) == 0; });
}

int player_stat(int stat)    { return ck::critter_base_stat(fallout::gDude, stat); }
int player_pc_stat(int stat) { return ck::critter_pc_stat(stat); }
int player_set_base_stat(int stat, int value) { return ck_critter_set_base_stat(fallout::gDude, stat, value); }
int player_set_bonus_stat(int stat, int value) { return ck_critter_set_bonus_stat(fallout::gDude, stat, value); }
int player_give_experience(int xp) { return fallout::pcAddExperience(xp); }

int ck_critter_get_hp(void* ptr) {
    return with_critter(ptr, 0, [](auto* c) { return ck::critter_get_hp(c); });
}

int ck_critter_get_max_hp(void* ptr) {
    return with_critter(ptr, 0, [](auto* c) { return fallout::critterGetStat(c, fallout::STAT_MAXIMUM_HIT_POINTS); });
}

int ck_critter_set_current_hp(void* ptr, int target_hp) {
    return with_critter(ptr, -1, [=](auto* c) { return ck::critter_adjust_hp(c, target_hp); });
}

int ck_critter_set_full_hp(void* ptr) {
    return with_critter(ptr, -1, [](auto* c) { return ck::critter_set_full_hp(c); });
}
