// src/object/ck_object.h
#ifndef CK_OBJECT_H
#define CK_OBJECT_H

#include "ck_api.h"
#include "geometry/geometry.h"
#include "ck_registry/ck_registry.h"

namespace fallout {
	struct Object;
}

const int BLOCKER_PID=0x2000158;  // dummy collision object
const int BLOCKER_FID=0x02000015;

struct LuaMeta;

namespace ck::object {
	void remove_at(int tile);
}

bool ck_object_blocking(int tile);

fallout::Object* ck_object_create(int pid, int tile);
fallout::Object* ck_object_create_at(int fid, int tile);

int ck_object_register_object_by_fid(int fid, int tile, const LuaMeta& meta);
int ck_object_register_object(int pid, int tile, const LuaMeta& meta);

void ck_object_remove(fallout::Object* obj);
void ck_object_remove_blocker_at(int tile);

void ck_object_create_blocker_at(int tile);

CK_API void ck_object_remove_at(int tile);
CK_API int ck_object_get_tile(int lua_id);
CK_API int ck_object_get_sid(int lua_id);
CK_API void* ck_object_get_ptr(int lua_id);

#endif
