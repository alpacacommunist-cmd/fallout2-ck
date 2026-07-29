#ifndef CK_DEBUG_OBJECT_FORMAT_H
#define CK_DEBUG_OBJECT_FORMAT_H

#include <string>
#include <vector>

#include "ck_debug_overlay/ck_debug_overlay_hexes.h"

extern "C" { int ck_critter_get_max_hp(void* ptr); }

int ck_map_get_floor_fid(int tile, int elevation);

namespace fallout {
    struct Object;
	extern int gElevation;
	int critterGetStat(Object* critter, int stat);

	bool isExitGridAt(int tile, int elevation);
	int tileGetHexGridWidth();

	Object* objectFindFirstAtLocation(int elevation, int tile);
	char* objectGetName(Object* obj);
	Object* objectFindNextAtLocation();
}

namespace ck::debug {
	void export_full_dump(const std::vector<ckDebugHex*>& hexes);
    void export_lua_tiles(const std::vector<ckDebugHex*>& hexes);

    std::string format_object_data(fallout::Object* obj, int obj_type);
}

#endif
