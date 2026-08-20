#include <algorithm>
#include "map/ck_map.h"
#include "map/ck_map_batch.h"
#include "ck_rendering.h"
#include "object/ck_object.h"

void ck_map_batch_tiles(const CkFFITile* tiles, int count) {
    if (count <= 0) return;

    for (int i = 0; i < count; ++i) {
        const auto& src = tiles[i];
        ck_rendering_add_tile(src.fid, src.tile);
    }
}

void ck_map_batch_roof_tiles(const CkFFITile *tiles, int count) {
    if (count <= 0) return;

    gRoofTiles.reserve(gRoofTiles.size() + count);

    for (int i = 0; i < count; ++i) {
        const auto &src = tiles[i];

        CkTileInstance instance;
        instance.tile = src.tile;
        instance.fid = src.fid;
        instance.roof_block_id = src.roof_block_id;
        instance.flags = 0;

        gRoofTiles.push_back(instance);
    }

    std::sort(gRoofTiles.begin(), gRoofTiles.end(),
        [](const CkTileInstance& a, const CkTileInstance& b) {
            return a.tile < b.tile;
        });
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

