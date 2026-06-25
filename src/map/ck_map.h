// src/map/ck_map.h
#ifndef CK_SCRIPTING_MAP_H
#define CK_SCRIPTING_MAP_H

#include "ck_api.h"
#include "object/ck_object_registry.h"

#include <string>

namespace ck {
	void on_map_enter();
	void on_before_map_enter();
}

namespace fallout {
	int mapGetCurrentMap();
}

void ck_map_add_scenery(const std::string& key, int tile);
void ck_map_add_scenery(int fid, int tile);

void ck_map_add_tile(int fid, int tile);
void ck_map_add_tile(const std::string& key, int tile);

struct CkCameraBorders {
    bool enabled = false;

    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
};

void ck_map_clear_camera_borders();
bool ck_map_is_camera_position_allowed(int tile);
bool ck_map_has_camera_borders();

const CkCameraBorders& ck_map_get_camera_borders();

CK_API int  ck_map_get_id();
CK_API void ck_map_add_scenery_fid(int fid, int tile);
CK_API void ck_map_add_scenery_key(const char* key, int tile);
CK_API void ck_map_add_tile_fid(int fid, int tile);
CK_API void ck_map_add_tile_key(const char* key, int tile);
CK_API void ck_map_set_camera_borders(int left, int right, int top, int bottom);
CK_API void ck_map_remove_blocker(int tile);
CK_API void ck_map_create_blocker(int tile);
CK_API void ck_map_create_object(int artId, int tile);
CK_API void ck_map_create_object_fid(int fid, int tile);
CK_API int  ck_map_register_object(int artId, int tile, const char* mod_id);

#endif
