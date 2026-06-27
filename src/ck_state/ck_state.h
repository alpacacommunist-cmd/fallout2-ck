// src/ck_state/ck_state.h
#ifndef CK_SCRIPTING_MAP_H
#define CK_SCRIPTING_MAP_H

#include "ck_api.h"

CK_API bool ck_state_load(const char* path);
void ck_state_save(const char* path);

#endif
