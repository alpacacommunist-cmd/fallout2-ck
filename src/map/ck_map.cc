#include "system/rendering.h"

#include "map/ck_map.h"
#include "map/ck_map_camera_borders.h"
#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "ck_mods/ck_mods.h"
#include "ck_gameplay_common.h"

namespace fallout {
	int squareTileFromTile(int tile);

	extern int* gMapLocalVars;
	extern int  gMapLocalVarsLength;
}

#include "log.h"
static constexpr logging::Context logger{"ck_config_maps.cc"};

namespace ck::map {
    bool map_has_camera_borders(int map_index) {
        return ck::map::borders::has_borders_for_map(map_index);
    }

    bool map_is_camera_position_allowed(int tile) {
        return ck::map::borders::is_camera_position_allowed(tile);
    }
}

static int square_tile_data_for_tile(int tile, int elevation) {
    if (!ck::common::hex_grid_tile_is_valid(tile) || elevation < 0 || elevation > 2) return -1;

    int square_tile = fallout::squareTileFromTile(tile);
    if (!ck::common::square_grid_tile_is_valid(square_tile)) return -1;

    return ck::common::square(elevation, square_tile);
}

// ref: [static void op_get_tile_fid(Program* program)]
int ck_map_get_floor_fid(int tile, int elevation) {
    int square_data = square_tile_data_for_tile(tile, elevation);
    int floor_index = square_data & 0x3FFF;

    return 0x04000000 | floor_index;
}

// ref: [static void op_get_tile_fid(Program* program)]
int ck_map_get_roof_fid(int tile, int elevation) {
    int square_data = square_tile_data_for_tile(tile, elevation);
    int roof_index = (square_data >> 16) & 0x3FFF;

    if (roof_index == 0) return -1;
    return 0x04000000 | roof_index; // just the fid
}

void ck_map_add_scenery(int fid, int tile) {
    ck_rendering_add_scenery(fid, tile, CkRenderLayer::Floor, 0);
}
void ck_map_add_tile(int fid, int tile) {
    ck_rendering_add_tile(fid, tile);
}

void ck_map_add_roof_scenery(int fid, int tile, int offset_y) {
    ck_rendering_add_scenery(fid, tile, CkRenderLayer::Roof, offset_y);
}
void ck_map_add_roof_tile(int fid, int tile, int roof_block_id) {
    ck_rendering_add_tile_roof(fid, tile, roof_block_id);
}

// FFI
void ck_map_add_scenery_fid(int fid, int tile) { ck_map_add_scenery(fid, tile); }
void ck_map_add_roof_scenery_fid(int fid, int tile, int offset_y) { ck_map_add_roof_scenery(fid, tile, offset_y); }

void ck_map_add_tile_fid(int fid, int tile) { ck_map_add_tile(fid, tile); }
void ck_map_add_roof_tile_fid(int fid, int tile, int roof_block_id ) { ck_map_add_roof_tile(fid, tile, roof_block_id); }

int ck_map_register_object(int pid, int tile) {
    const LuaMeta& meta  = { ck::mods::current_mod_id(), {}, {}, {} };

    return ck_object_register(pid, tile, meta);
}

int ck_map_create_blocker_at(int tile) {
    const LuaMeta& meta = { ck::mods::current_mod_id(), {}, {}, {} };

    return ck_object_register(BLOCKER_PID, tile, meta);
}

int ck_map_get_mvar(int index) {
    if (fallout::gMapLocalVars == nullptr || index < 0 || index >= fallout::gMapLocalVarsLength) return 0;
    return fallout::gMapLocalVars[index];
}

void ck_map_set_mvar(int index, int value) {
    if (fallout::gMapLocalVars == nullptr || index < 0 || index >= fallout::gMapLocalVarsLength) return;
    fallout::gMapLocalVars[index] = value;
}

bool ck_tile_is_blocked(int tile, int elevation) { return ck_object_blocking(tile, elevation); }
