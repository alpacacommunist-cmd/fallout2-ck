// src/object/ck_object.h
#ifndef CK_SCRIPTING_OBJECT_H
#define CK_SCRIPTING_OBJECT_H

#include "ck_api.h"
#include "geometry/geometry.h"
#include "object/ck_object_registry.h"
#include "object.h"

static fallout::Object* ck_object_blocker_at(int tile);
bool ck_object_blocking(int tile);

fallout::Object* ck_object_create_at(int fid, int tile);
fallout::Object* ck_object_create_critter(int pid, int tile);

int ck_object_register_object(int pid, int tile);
int ck_object_register_critter(int pid, int tile, const LuaCritterMeta& meta);

void ck_object_remove_blocker_at(int tile);
void ck_object_create_blocker_at(int tile);

void ck_object_remove_all_by_pid(int pid);
void ck_object_create_pid_at(int pid, int tile);

void ck_object_toggle_visibility_in_rect(const HexRect& rect, bool visible);

CK_API void ck_map_remove_all_by_pid(int pid);
CK_API void ck_map_create_pid_at(int pid, int tile);
#endif
