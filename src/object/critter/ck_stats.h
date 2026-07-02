#ifndef CK_STATS_H
#define CK_STATS_H

#include "ck_api.h"
#include "stat_defs.h"

namespace fallout {
	struct Object;
	extern Object* gDude;

	int critterGetStat(Object* critter, int stat);
	int pcGetStat(int pcStat);

	int critterGetHitPoints(Object* critter);
	int critterAdjustHitPoints(Object* critter, int hp);
	int critterSetBaseStat(Object* critter, int stat, int value);
}

namespace ck {
	int critter_stat(fallout::Object* critter, int stat);
	int critter_pc_stat(int stat);
	int critter_set_base_stat(fallout::Object* critter, int stat, int value);
}

CK_API int player_stat(int stat);
CK_API int player_pc_stat(int stat);

CK_API bool ck_critter_set_base_stat(void* ptr, int stat, int value);
CK_API int ck_critter_get_stat(void* ptr, int stat_id);
CK_API void ck_critter_set_current_hp(void* ptr, int target_hp);

CK_API void ck_get_stats_metadata(void (*callback)(const char* lua_name, int value));
CK_API void ck_get_pc_stats_metadata(void (*callback)(const char* lua_name, int value));

#endif
