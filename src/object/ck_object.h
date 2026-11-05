// src/object/ck_object.h
#ifndef CK_OBJECT_H
#define CK_OBJECT_H

#include "ck_api.h"

namespace fallout {
#define HEX_GRID_WIDTH (200)
#define HEX_GRID_HEIGHT (200)
#define HEX_GRID_SIZE (HEX_GRID_WIDTH * HEX_GRID_HEIGHT)
	struct Object;
    enum Rotation : int;

	extern int gElevation;

	char* objectGetName(Object* obj);
    int tileGetTileInDirection(int tile, Rotation rotation, int distance);
}

inline bool hexGridTileIsValid(int tile) {
	return tile >= 0 && tile < HEX_GRID_SIZE;
}

const int BLOCKER_PID=0x2000158;  // dummy collision object
const int BLOCKER_FID=0x02000015;
struct LuaMeta;

namespace ck::object {
	void remove_at(int tile);
	void destroy_at(int tile);

	int find_at(int tile, fallout::Object** buffer, int max_count);
	int find_by_pid(int pid, fallout::Object** buffer, int max_count);
    fallout::Object* find_id_at(int tile, int elevation, int id);

    int type(int pid);
    int type(fallout::Object* object);
}

bool ck_object_blocking(int tile, int elevation = fallout::gElevation);

fallout::Object* ck_object_create(int pid, int tile, int elevation = fallout::gElevation, bool search_free_tile = false);
fallout::Object* ck_object_create_at(int fid, int tile);

void ck_object_remove(fallout::Object* obj);
void ck_object_remove_blocker_at(int tile);

void ck_object_create_blocker_at(int tile);

CK_API void ck_object_remove_at(int tile);

CK_API char* ck_object_get_name(fallout::Object* object);
CK_API char* ck_object_get_description(fallout::Object* object);
CK_API int ck_object_get_type(void* ptr);

CK_API int ck_object_find_at_tile(int tile, fallout::Object** buffer, int max_count);
CK_API int ck_object_find_by_pid(int pid, fallout::Object** buffer, int max_count);

CK_API int player_get_pid();
CK_API fallout::Object* player_get_ptr();

#endif
