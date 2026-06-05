#include <iostream>

#include "art.h"
#include "debug.h"
#include "light.h"
#include "mouse.h"
#include "object.h"
#include "tile.h"

#include "ck_rendering.h"

// frame
void ck_rendering_draw_scenery(int fid, int x, int y) {
    gSceneryDrawRequests.push_back({ fid, x, y });
}

// persistent
void ck_rendering_add_scenery(int fid, int tile) {
    gPersistentScenery.push_back({ fid, tile });
}

void ck_rendering_add_tile(int fid, int tile) {
    gPersistentTiles.push_back({ fid, tile });
}

void ck_rendering_clear() {
	gPersistentScenery.clear();
	gPersistentTiles.clear();
}

using namespace fallout;

// frame queue
static void ck_rendering_draw(fallout::Rect* rect);
// persistent queue
static void ck_rendering_scenery(fallout::Rect* rect);
static void ck_rendering_tiles(fallout::Rect* rect);

void ck_rendering_render(fallout::Rect* rect) {
	ck_rendering_scenery(rect);
	ck_rendering_tiles(rect);

	ck_rendering_draw(rect);
}


int ck_rendering_build_scenery_fid(int fid) {
    return buildFid(OBJ_TYPE_SCENERY, fid, 0, 0, 0);
}

int ck_rendering_build_tile_fid(int fid) {
    return buildFid(OBJ_TYPE_TILE, fid, 0, 0, 0);
}

static void draw_scenery_art(int fid, int x, int y, Rect* rect) {
    CacheEntry* cacheEntry;
    Art* art = artLock(fid, &cacheEntry);

    if (art == nullptr) return;

    int width = artGetWidth(art, 0, 0);
    int height = artGetHeight(art, 0, 0);

    Rect artRect;
    artRect.left = x, artRect.top = y, artRect.right = x + width - 1, artRect.bottom = y + height - 1;

    Rect intersection;
    if (rectIntersection(&artRect, rect, &intersection) == -1) {
		artUnlock(cacheEntry);
        return;
    }

    unsigned char* src = artGetFrameData(art, 0, 0);

    src += width * (intersection.top - y) + (intersection.left - x);

	int light = lightGetAmbientIntensity();
	_dark_trans_buf_to_buf(src, rectGetWidth(&intersection), rectGetHeight(&intersection),
			width, tileGetWindowBuffer(), intersection.left, intersection.top,
			tileGetWindowPitch(), light);

    artUnlock(cacheEntry);
}

static void ck_rendering_tiles(fallout::Rect* rect) {
    for (const auto& tileInstance : gPersistentTiles) {
        int screenX, screenY;
        tileToScreenXY(tileInstance.tile, &screenX, &screenY);

        int fid = ck_rendering_build_tile_fid(tileInstance.fid);
        tileRenderFloorExternal(fid, screenX, screenY, rect);
    }
}

static void ck_rendering_scenery(fallout::Rect* rect) {
    for (const auto& scenery : gPersistentScenery) {
        int screenX, screenY;
        tileToScreenXY(scenery.tile, &screenX, &screenY);

        int fid = ck_rendering_build_scenery_fid(scenery.fid);
        draw_scenery_art(fid, screenX, screenY, rect);
    }
}

// refactor zone

static void ck_rendering_draw(fallout::Rect* rect) {
	int anchor_tile = 17290;

	int screen_x;
	int screen_y;

	tileToScreenXY(anchor_tile, &screen_x, &screen_y);

	// debugPrint("scenery requests: %d\n", gSceneryDrawRequests.size());

	for (const auto& scenery : gSceneryDrawRequests) {
		int fid = buildFid(OBJ_TYPE_SCENERY, scenery.fid, 0, 0, 0);

		draw_scenery_art(fid, screen_x + scenery.x, screen_y + scenery.y, rect);
	}

	gSceneryDrawRequests.clear();
}

