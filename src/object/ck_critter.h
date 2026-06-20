// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"

namespace ck {
	fallout::Object* create_critter(int pid, int tile);
	int register_critter(int pid, int tile);
}

CK_API int ck_critter_register(int pid, int tile);

#endif
