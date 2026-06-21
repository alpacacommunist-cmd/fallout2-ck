// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"

namespace ck {
	fallout::Object* create_critter(int pid, int tile);
	int register_critter(int pid, int tile);

	int critter_stat(fallout::Object* critter, int stat);
	int critter_pc_stat(int stat);
}

CK_API int ck_critter_register(int pid, int tile);
CK_API int player_stat(int stat);
CK_API int player_pc_stat(int stat);

#endif
