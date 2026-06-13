// src/object/ck_object.h
#ifndef CK_SCRIPTING_OBJECT_H
#define CK_SCRIPTING_OBJECT_H

#include "object.h"

static fallout::Object* ck_object_blocker_at(int tile);
bool ck_object_blocking(int tile);
void ck_object_remove_blocker_at(int tile);
void ck_object_create_blocker_at(int tile);
void ck_object_create_at(int fid, int tile);

void ck_object_critter_create(int pid, int tile, int lua_script_id);
#endif
