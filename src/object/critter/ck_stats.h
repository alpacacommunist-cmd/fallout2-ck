#ifndef CK_STATS_H
#define CK_STATS_H

#include "ck_api.h"
#include "stat_defs.h"

namespace fallout {
	struct Object;
	extern Object* gDude;

	enum Stat : int;
	enum PcStat : int;

	int critterGetStat(Object* critter, Stat stat);
	int pcGetStat(PcStat pcStat);

	int critterGetHitPoints(Object* critter);
	int critterAdjustHitPoints(Object* critter, int hp);
	int critterSetBaseStat(Object* critter, Stat stat, int value);

	int critterGetBonusStat(Object* critter, Stat stat);
	int critterSetBonusStat(Object* critter, Stat stat, int value);
    int pcAddExperience(int xp, int* xpGained = nullptr);
}

namespace ck {
	int critter_base_stat(fallout::Object* critter, int stat);
	int critter_set_base_stat(fallout::Object* critter, int stat, int value);
	int critter_bonus_stat(fallout::Object* critter, int stat);
	int critter_set_bonus_stat(fallout::Object* critter, int stat, int value);
	int critter_pc_stat(int stat);

	int critter_get_hp(fallout::Object* critter);
	int critter_get_max_hp(fallout::Object* critter);

	int critter_adjust_hp(fallout::Object* critter, int target_hp);
	int critter_set_full_hp(fallout::Object* critter);
}

CK_API void ck_get_stats_metadata(void (*callback)(const char* lua_name, int value));
CK_API void ck_get_pc_stats_metadata(void (*callback)(const char* lua_name, int value));

CK_API int ck_critter_get_base_stat(void* ptr, int stat_id);
CK_API bool ck_critter_set_base_stat(void* ptr, int stat, int value);
CK_API int ck_critter_get_bonus_stat(void* ptr, int stat);
CK_API bool ck_critter_set_bonus_stat(void* ptr, int stat, int value);
CK_API int player_stat(int stat);
CK_API int player_pc_stat(int stat);
CK_API int player_set_base_stat(int stat, int value);
CK_API int player_set_bonus_stat(int stat, int value);
CK_API int player_give_experience(int xp);

CK_API int ck_critter_get_hp(void* ptr);
CK_API int ck_critter_get_max_hp(void* ptr);
CK_API int ck_critter_set_current_hp(void* ptr, int target_hp);
CK_API int ck_critter_set_full_hp(void* ptr);

#endif
