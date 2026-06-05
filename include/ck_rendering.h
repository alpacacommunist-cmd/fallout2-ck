#ifndef CK_RENDERING_H
#define CK_RENDERING_H

#include <vector>

namespace fallout {
	struct Rect;
}

void ck_rendering_render(fallout::Rect* rect);
void ck_rendering_clear();

void ck_rendering_draw_scenery(int fid, int x, int y);

// frame queue
struct CkSceneryDrawRequest { int fid; int x; int y; };
static std::vector<CkSceneryDrawRequest> gSceneryDrawRequests;

// persistent queues
struct CkSceneryInstance { int fid; int tile; };
static std::vector<CkSceneryInstance> gPersistentScenery;

struct CkTileInstance { int fid; int tile; };
static std::vector<CkTileInstance> gPersistentTiles;

const std::vector<CkSceneryInstance>& ck_rendering_get_scenery();
const std::vector<CkTileInstance>& ck_rendering_get_tiles();

void ck_rendering_add_scenery(int fid, int tile);
void ck_rendering_add_tile(int fid, int tile);

int ck_rendering_build_tile_fid(int fid);
int ck_rendering_build_scenery_fid(int fid);

#endif
