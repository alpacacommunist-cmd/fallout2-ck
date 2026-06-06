#include <iostream>
#include <unordered_map>

#include "art.h"
#include "debug.h"
#include "light.h"
#include "mouse.h"
#include "object.h"
#include "tile.h"

#include "ck_rendering.h"

struct CachedArt {
    fallout::Art* art = nullptr;
    fallout::CacheEntry* cacheEntry = nullptr;
    int width = 0;
    int height = 0;
    unsigned char* frameData = nullptr;
};

// global texture cache
static std::unordered_map<int, CachedArt> gArtCache;

static const CachedArt* get_or_cache_art(int fid) {
    auto it = gArtCache.find(fid);
    if (it != gArtCache.end()) {
        return &(it->second);
    }

    fallout::CacheEntry* entry = nullptr;
    fallout::Art* art = fallout::artLock(fid, &entry);

    if (art == nullptr) return nullptr;

    CachedArt cached;
    cached.art = art;
    cached.cacheEntry = entry;
    cached.width = fallout::artGetWidth(art, 0, 0);
    cached.height = fallout::artGetHeight(art, 0, 0);
    cached.frameData = fallout::artGetFrameData(art, 0, 0);

    gArtCache[fid] = cached;
    return &gArtCache[fid];
}

void ck_rendering_clear_art_cache() {
    for (auto& [fid, cached] : gArtCache) {
        if (cached.cacheEntry) {
            fallout::artUnlock(cached.cacheEntry);
        }
    }
    gArtCache.clear();
}

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

	ck_rendering_clear_art_cache();
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

static void draw_scenery_art(int fid, int x, int y, fallout::Rect* rect) {
    // Получаем арт из нашего быстрого кэша указателей
    const CachedArt* cached = get_or_cache_art(fid);
    if (cached == nullptr || cached->frameData == nullptr) return;

    fallout::Rect artRect;
    artRect.left = x; artRect.top = y; artRect.right = x + cached->width - 1; artRect.bottom = y + cached->height - 1;

    fallout::Rect intersection;
	// object is out of screen
    if (fallout::rectIntersection(&artRect, rect, &intersection) == -1) return;

    unsigned char* src = cached->frameData;
    src += cached->width * (intersection.top - y) + (intersection.left - x);

    int light = fallout::lightGetAmbientIntensity();
    fallout::_dark_trans_buf_to_buf(
        src, 
        fallout::rectGetWidth(&intersection), 
        fallout::rectGetHeight(&intersection),
        cached->width, 
        fallout::tileGetWindowBuffer(), 
        intersection.left, 
        intersection.top,
        fallout::tileGetWindowPitch(), 
        light
    );
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

