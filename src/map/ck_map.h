// src/map/ck_map.h
#ifndef CK_SCRIPTING_MAP_H
#define CK_SCRIPTING_MAP_H

#include <string>

void ck_scripting_on_map_enter();

void ck_map_add_scenery(const std::string& key, int tile);
void ck_map_add_scenery(int fid, int tile);
void ck_map_add_tile(int fid, int tile);
void ck_map_remove_blocker(int tile);
void ck_map_create_blocker(int tile);
void ck_map_create_object(int fid, int tile);

struct CkCameraBorders {
    bool enabled = false;

    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
};

void ck_map_set_camera_borders(int left, int right, int top, int bottom);
void ck_map_clear_camera_borders();
bool ck_map_is_camera_position_allowed(int tile);
bool ck_map_has_camera_borders();

const CkCameraBorders& ck_map_get_camera_borders();

#endif
