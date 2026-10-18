#ifndef CK_DEBUG_OBJECT_FORMAT_H
#define CK_DEBUG_OBJECT_FORMAT_H

#include "debug/hexes.h"

extern "C" { int ck_critter_get_max_hp(void* ptr); }

int ck_map_get_floor_fid(int tile, int elevation);
int ck_map_get_roof_fid(int tile, int elevation);

namespace fallout {
    struct Object;
}

namespace ck::debug {
	void export_full_dump(const std::vector<ckDebugHex*>& hexes);
    void export_lua_tiles(const std::vector<ckDebugHex*>& hexes);

    std::string format_object_data(fallout::Object* obj, int obj_type);
}

#endif
