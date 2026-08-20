#ifndef CK_RENDERING_H
#define CK_RENDERING_H

#include "ck_api.h"
#include <vector>

namespace fallout {
	struct Rect;
}

enum class CkRenderLayer {
    Floor,
    Roof
};

struct CkSceneryInstance {
    int tile;
    int offset_y = 0;
    int fid = -1;
};

struct CkTileInstance {
    int tile;
    int offset_y = 0;
    int fid = -1;
    int roof_block_id = -1;
    int flags = 0;
};

extern std::vector<CkSceneryInstance> gScenery;
extern std::vector<CkTileInstance> gTiles;

extern std::vector<CkSceneryInstance> gRoofScenery;
extern std::vector<CkTileInstance> gRoofTiles;

const std::vector<CkSceneryInstance>& ck_rendering_get_scenery();
const std::vector<CkTileInstance>& ck_rendering_get_tiles();

void ck_rendering_add_scenery(int fid, int tile, CkRenderLayer layer, int offset_y = 0);
void ck_rendering_add_tile(int fid, int tile);
void ck_rendering_add_tile_roof(int fid, int tile, int roof_block_id);

namespace ck::rendering {
    void floor(fallout::Rect* rect);
    void roof(fallout::Rect* rect);
}

CK_API void ck_rendering_clear();
CK_API void ck_rendering_refresh();

#endif
