#include <algorithm>

#include "ck_rendering.h"
#include "map/ck_map.h"
#include "map/ck_map_camera_borders.h"
#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "ck_proto/ck_proto_registry.h"

#include "map_defs.h"

#include "ck_log.h"
static const Logger log("CK Map");

namespace fallout {
    struct TileData { int field_0[SQUARE_GRID_SIZE]; };
    extern TileData* _square[ELEVATION_COUNT];
}

namespace ck {
    void on_map_enter() {
        ck::proto::sync_custom_items_on_map(ck::proto::SyncMode::Restore);

        ck_dispatcher_on_map_enter();
        ck_rendering_refresh();

        if (ck_in_combat()) fallout::_combat_reload_map();
    }

    void on_before_map_load() {
        log.debug("on_before_map_load");

        ck::reset_dummy_script();
        ck::registry::on_map_exit();

        fallout::mapEdgeFree();
        ck_rendering_clear();

        if (ck_debug_overlay_enabled()) ck_debug_overlay_toggle();
    }

    int current_map_id() { return static_cast<int>(fallout::mapGetCurrentMap()); }

    bool map_has_camera_borders(int map_index) {
        return ck::map::borders::has_borders_for_map(map_index);
    }

    bool map_is_camera_position_allowed(int tile) {
        return ck::map::borders::is_camera_position_allowed(tile);
    }
}

int ck_map_get_floor_fid(int tile, int elevation) {
    if (!fallout::hexGridTileIsValid(tile) || elevation < 0 || elevation >= 3) return -1;

    int squareTile = fallout::squareTileFromTile(tile);
    if (!fallout::squareGridTileIsValid(squareTile)) return -1;

    int squareData  = fallout::_square[elevation]->field_0[squareTile];
    int floor_index = squareData & 0x3FFF;

    return 0x04000000 | floor_index;
}

int get_roof_fid(int tile, int elevation) {
    if (!fallout::hexGridTileIsValid(tile) || elevation < 0 || elevation >= 3) return -1;

    int squareTile = fallout::squareTileFromTile(tile);
    if (!fallout::squareGridTileIsValid(squareTile)) return -1;

    if (fallout::_square[elevation] == nullptr) return -1;

    int squareData = fallout::_square[elevation]->field_0[squareTile];
    int roof_index = (squareData >> 16) & 0x3FFF;

    if (roof_index == 0) return -1;

    return 0x01000000 | roof_index;
}

void ck_map_add_scenery(int fid, int tile) {
    ck_rendering_add_scenery(fid, tile);
}

void ck_map_add_tile(int fid, int tile) {
    ck_rendering_add_tile(fid, tile);
}

// ffi

int ck_map_get_id() {
    return ck::current_map_id();
}

void ck_map_add_scenery_fid(int fid, int tile) {
    ck_map_add_scenery(fid, tile);
}

void ck_map_add_tile_fid(int fid, int tile) {
    ck_rendering_add_tile(fid, tile);
}

int ck_map_register_object(int pid, int tile) {
    const LuaMeta& meta  = { ck_get_current_mod_id(), {}, {}, {} };

    return ck_object_register(pid, tile, meta);
}

int ck_map_create_blocker_at(int tile) {
    const LuaMeta& meta = { ck_get_current_mod_id(), {}, {}, {} };

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

void ck_map_batch_tiles(const CkFFITile* tiles, int count) {
    for (int i = 0; i < count; ++i) {
        const auto& src = tiles[i];

        ck_rendering_add_tile(src.fid, src.tile);
    }
}

void ck_map_batch_scenery(const CkFFIScenery* sceneries, int count) {
    if (count <= 0) return;

    gScenery.reserve(gScenery.size() + count);

    for (int i = 0; i < count; ++i) {
        const auto& src = sceneries[i];

        CkSceneryInstance instance;
        instance.tile = src.tile;
        instance.fid = src.fid;

        gScenery.push_back(instance);
    }

    std::sort(gScenery.begin(), gScenery.end(),
              [](const CkSceneryInstance& a, const CkSceneryInstance& b) { return a.tile < b.tile; });
}

void ck_map_batch_blockers(const CkFFIBlocker* blockers, int count) {
    for (int index = 0; index < count; ++index) {
        const auto& src = blockers[index];

        if (src.tile != -1) ck_map_create_blocker_at(src.tile);
    }
}

void ck_map_batch_clear(const CkFFIClear* tiles, int count) {
    for (int index = 0; index < count; ++index) {
        const auto& src = tiles[index];

        if (src.tile != -1) ck::object::remove_at(src.tile);
    }
}

bool ck_tile_is_blocked(int tile, int elevation) {
    return ck_object_blocking(tile, elevation);
}

int ck_current_elevation() {
    return fallout::gElevation;
}
