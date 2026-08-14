#include "perk_defs.h"

#include "ck_log.h"
static const Logger log("CK Perks");

static const char* g_perk_names[] = {
    "awareness", "bonus_hth_attacks", "bonus_hth_damage", "bonus_move", "bonus_ranged_damage", "bonus_rate_of_fire",
    "earlier_sequence", "faster_healing", "more_criticals", "night_vision", "presence", "rad_resistance", "toughness",
    "strong_back", "sharpshooter", "silent_running", "survivalist", "master_trader", "educated",
    "healer", "fortune_finder", "better_criticals", "empathy", "slayer", "sniper", "silent_death", "action_boy",
    "mental_block", "lifegiver", "dodger", "snakeater", "mr_fixit", "medic", "master_thief", "speaker", "heave_ho",
    "friendly_foe", "pickpocket", "ghost", "cult_of_personality", "scrounger", "explorer", "flower_child", "pathfinder",
    "animal_friend", "scout", "mysterious_stranger", "ranger", "quick_pockets", "smooth_talker", "swift_learner", "tag",
    "mutate", "nuka_cola_addiction", "buffout_addiction", "mentats_addiction", "psycho_addiction", "radaway_addiction",
    "weapon_long_range", "weapon_accurate", "weapon_penetrate", "weapon_knockback", "powered_armor",
    "combat_armor", "weapon_scope_range", "weapon_fast_reload", "weapon_night_sight", "weapon_flameboy",
    "armor_advanced_i", "armor_advanced_ii", "jet_addiction", "tragic_addiction", "armor_charisma", "gecko_skinning",
    "dermal_impact_armor", "dermal_impact_assault_enhancement", "phoenix_armor_implants", "phoenix_assault_enhancement",
    "vault_city_inoculations", "adrenaline_rush", "cautious_nature", "comprehension", "demolition_expert",
    "gambler", "gain_strength", "gain_perception", "gain_endurance", "gain_charisma", "gain_intelligence", "gain_agility",
    "gain_luck", "harmless", "here_and_now", "hth_evade", "kama_sutra_master", "karma_beacon", "light_step",
    "living_anatomy", "magnetic_personality", "negotiator", "pack_rat", "pyromaniac", "quick_recovery", "salesman", "stonewall",
    "thief", "weapon_handling", "vault_city_training",
    "alcohol_raised_hit_points", "alcohol_raised_hit_points_ii", "alcohol_lowered_hit_points", "alcohol_lowered_hit_points_ii",
    "autodoc_raised_hit_points", "autodoc_raised_hit_points_ii", "autodoc_lowered_hit_points", "autodoc_lowered_hit_points_ii",
    "expert_excrement_expeditor", "weapon_enhanced_knockout", "jinxed",
};

static_assert(sizeof(g_perk_names) / sizeof(g_perk_names[0]) == fallout::Perk::PERK_COUNT,
              "perk names / perk_defs.h mismatch!");

namespace ck::perks {
}

void ck_get_perks_metadata(void (*callback)(const char* name, int value)) {
    for (int i = 0; i < fallout::Perk::PERK_COUNT; ++i) callback(g_perk_names[i], i);
}

