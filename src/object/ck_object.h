// src/object/ck_object.h
#ifndef CK_OBJECT_H
#define CK_OBJECT_H

#include "ck_api.h"
#include "geometry/geometry.h"
#include "object/ck_object_registry.h"
#include "object.h"

static fallout::Object* ck_object_blocker_at(int tile);

bool ck_object_blocking(int tile);

fallout::Object* ck_object_create(int pid, int tile);
fallout::Object* ck_object_create_at(int fid, int tile);

int ck_object_register_object(int pid, int tile, const LuaMeta& meta);

void ck_object_remove(fallout::Object* obj);
void ck_object_remove_at(int tile);
void ck_object_remove_blocker_at(int tile);

void ck_object_create_blocker_at(int tile);

CK_API int ck_object_get_tile(int lua_id);
CK_API int ck_object_get_sid(int lua_id);
CK_API void* ck_object_get_ptr(int lua_id);

#endif
