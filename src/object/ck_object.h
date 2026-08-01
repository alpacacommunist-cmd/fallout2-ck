// src/object/ck_object.h
#ifndef CK_OBJECT_H
#define CK_OBJECT_H

#include "ck_api.h"

namespace fallout {
	struct Object;

	extern int gElevation;

	char* objectGetName(Object* obj);
}

const int BLOCKER_PID=0x2000158;  // dummy collision object
const int BLOCKER_FID=0x02000015;
struct LuaMeta;

struct CkObjectFFI {
    void*     c_ptr = nullptr;
    int       id        = 0;
    int       pid       = 0;
    int       sid       = -1;
    int       tile      = -1;
    int       elevation = 0;
    int       flags     = 0;
    int       rotation  = 0;

	const char* name;

	int         lua_id    = -1;
	const char* mod_id = "__ck_system__";
};

namespace ck::object {
	void remove_at(int tile);
	int find_at_tile(int tile, CkObjectFFI* buffer, int max_count);
	int find_by_pid(int pid, CkObjectFFI* buffer, int max_count);
}

bool ck_object_blocking(int tile);

fallout::Object* ck_object_create(int pid, int tile);
fallout::Object* ck_object_create_at(int fid, int tile);

int ck_object_register(int pid, int tile, const LuaMeta& meta);

void ck_object_remove(fallout::Object* obj);
void ck_object_remove_blocker_at(int tile);

void ck_object_create_blocker_at(int tile);

CK_API void ck_object_remove_at(int tile);

CK_API void* ck_object_get_ptr(int lua_id);
CK_API int   ck_object_get_id(void* ptr);
CK_API int   ck_object_get_tile(void* ptr);
CK_API int   ck_object_get_sid(void* ptr);
CK_API char* ck_object_get_name(void* ptr);

CK_API int ck_object_find_at_tile(int tile, CkObjectFFI* buffer, int max_count);
CK_API int ck_object_find_by_pid(int pid, CkObjectFFI* buffer, int max_count);

#endif
