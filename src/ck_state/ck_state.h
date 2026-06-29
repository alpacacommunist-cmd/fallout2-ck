// src/ck_state/ck_state.h
#ifndef CK_STATE_H
#define CK_STATE_H

#include "ck_api.h"

int ck_dispatcher_get_sync_load_ref();
int ck_dispatcher_get_sync_save_ref();

CK_API bool ck_state_load(const char* path);
CK_API void ck_state_save(const char* path);

#endif
