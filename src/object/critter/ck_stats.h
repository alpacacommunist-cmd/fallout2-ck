#ifndef CK_STATS_H
#define CK_STATS_H

#include "ck_api.h"

namespace fallout {
	struct Object;
	extern Object* gDude;

	int critterGetStat(Object* critter, int stat);
	int pcGetStat(int pcStat);
}

namespace ck {
	int critter_stat(fallout::Object* critter, int stat);
	int critter_pc_stat(int stat);
}

CK_API int player_stat(int stat);
CK_API int player_pc_stat(int stat);
CK_API void ck_get_stats_metadata(void (*callback)(const char* lua_name, int value));
CK_API void ck_get_pc_stats_metadata(void (*callback)(const char* lua_name, int value));

#endif
