#ifndef CK_RENDERING_H
#define CK_RENDERING_H

#include "ck_api.h"
#include <vector>
#include <string>

namespace fallout {
	struct Rect;
}

void ck_rendering_render(fallout::Rect* rect);

struct CkSceneryInstance {
    int tile;
    int engineFid = -1;
    std::string assetKey;

    bool isCustomAsset() const { return engineFid == -1; }
};

struct CkTileInstance {
    int tile;
    int engineFid = -1;
    std::string assetKey;

    bool isCustomAsset() const { return engineFid == -1; }
};

static std::vector<CkSceneryInstance> gPersistentScenery;
static std::vector<CkTileInstance> gPersistentTiles;

const std::vector<CkSceneryInstance>& ck_rendering_get_scenery();
const std::vector<CkTileInstance>& ck_rendering_get_tiles();

void ck_rendering_add_custom_scenery(const std::string& key, int tile);
void ck_rendering_add_scenery(int fid, int tile);

void ck_rendering_add_custom_tile(const std::string& key, int tile);
void ck_rendering_add_tile(int fid, int tile);

int ck_rendering_build_tile_fid(int fid);
int ck_rendering_build_scenery_fid(int fid);

CK_API void ck_rendering_clear();
CK_API void ck_rendering_refresh();

#endif
