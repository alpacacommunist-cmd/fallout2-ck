#ifndef CK_STATS_H
#define CK_STATS_H

#include "stat.h"
#include "ck_api.h"

namespace ck {
	int critter_stat(fallout::Object* critter, int stat);
	int critter_pc_stat(int stat);
}

namespace fallout {
	extern Object* gDude;
}

CK_API int player_stat(int stat);
CK_API int player_pc_stat(int stat);

#endif
