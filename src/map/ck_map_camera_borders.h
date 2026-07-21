#ifndef CK_MAP_CAMERA_BORDERS_H
#define CK_MAP_CAMERA_BORDERS_H

#include "ck_api.h"

struct CkCameraBorders {
    int left   = 0;
    int right  = 0;
    int top    = 0;
    int bottom = 0;
};

namespace fallout {
	int tileGetHexGridWidth();
}

namespace ck::map::borders {
    void clear();
    void clear_for_mod();

    bool has_borders_for_map(int map_id);
    bool is_camera_position_allowed(int tile);
}

bool ck_map_has_camera_borders(int map_id);
void ck_map_clear_camera_borders();

CkCameraBorders ck_map_get_camera_borders(int map_id);

CK_API void ck_map_set_camera_borders(int map_id, const CkCameraBorders* borders);
CK_API void ck_map_clear_camera_borders_for_mod(const char* mod_id);

#endif // CK_MAP_CAMERA_BORDERS_H
