// src/map/ck_map.h
#ifndef CK_MAP_H
#define CK_MAP_H

#include "ck_api.h"

#include <string>

extern "C" const char* ck_get_current_mod_id();
extern "C" bool        ck_in_combat();
void ck_dispatcher_on_map_enter();

bool ck_debug_overlay_enabled();
void ck_debug_overlay_toggle();

namespace ck {
	void reset_dummy_script();

	void on_map_enter();
	void on_before_map_enter();


	int current_map_id();
	bool map_has_camera_borders(int map_index);
	bool map_is_camera_position_allowed(int tile);
}

namespace fallout {
	int  mapGetCurrentMap();
	bool _combat_reload_map();

	// tile.h
	int squareTileFromTile(int tile);

	extern int* gMapLocalVars;
	extern int  gMapLocalVarsLength;
}

int  ck_map_get_floor_fid(int tile, int elevation);
void ck_map_add_scenery(int fid, int tile);

void ck_map_add_tile(int fid, int tile);

CK_API int  ck_map_get_id();
CK_API void ck_map_add_scenery_fid(int fid, int tile);
CK_API void ck_map_add_tile_fid(int fid, int tile);
CK_API int  ck_map_create_blocker_at(int tile);
CK_API void ck_map_create_object(int artId, int tile);
CK_API void ck_map_create_object_fid(int fid, int tile);
CK_API int  ck_map_register_object(int artId, int tile);

CK_API int  ck_map_get_mvar(int index);
CK_API void ck_map_set_mvar(int index, int value);

struct CkFFITile {
    int tile;
    int fid;
};

struct CkFFIScenery { int tile; int fid; };
struct CkFFIBlocker { int tile; int fid; };
struct CkFFIClear { int tile; };

CK_API void ck_map_batch_tiles(const CkFFITile* tiles, int count);
CK_API void ck_map_batch_scenery(const CkFFIScenery* sceneries, int count);
CK_API void ck_map_batch_blockers(const CkFFIBlocker* blockers, int count);
CK_API void ck_map_batch_clear(const CkFFIClear* tiles, int count);


#endif
