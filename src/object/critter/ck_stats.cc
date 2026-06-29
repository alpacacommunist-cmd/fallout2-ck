#include "object/critter/ck_stats.h"

#include "ck_log.h"
static const Logger log("CK Stats");

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

