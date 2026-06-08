#include <unordered_map>

#include "art.h"
#include "debug.h"
#include "light.h"
#include "mouse.h"
#include "object.h"
#include "tile.h"

#include "ck_rendering.h"
#include "ck_assets/ck_asset_registry.h"

extern CkAssetRegistry gAssetRegistry;

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
    if (it != gArtCache.end()) return &(it->second);

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

void ck_rendering_add_scenery(int fid, int tile) {
    CkSceneryInstance inst;
    inst.tile      = tile;
    inst.engineFid = fid;
    gPersistentScenery.push_back(inst);
}

void ck_rendering_add_custom_scenery(const std::string& key, int tile) {
    CkSceneryInstance inst;
    inst.tile     = tile;
    inst.assetKey = key;
    gPersistentScenery.push_back(inst);
}

void ck_rendering_add_tile(int fid, int tile) {
    CkTileInstance inst;
    inst.tile      = tile;
    inst.engineFid = fid;
    gPersistentTiles.push_back(inst);
}

void ck_rendering_add_custom_tile(const std::string& key, int tile) {
    CkTileInstance inst;
    inst.tile     = tile;
    inst.assetKey = key;
    gPersistentTiles.push_back(inst);
}

void ck_rendering_clear() {
	gPersistentScenery.clear();
	gPersistentTiles.clear();

	ck_rendering_clear_art_cache();
}

using namespace fallout;

static void ck_rendering_scenery(fallout::Rect* rect);
static void ck_rendering_tiles(fallout::Rect* rect);

void ck_rendering_render(fallout::Rect* rect) {
	ck_rendering_scenery(rect);
	ck_rendering_tiles(rect);
}


int ck_rendering_build_scenery_fid(int fid) {
    return buildFid(OBJ_TYPE_SCENERY, fid, 0, 0, 0);
}

int ck_rendering_build_tile_fid(int fid) {
    return buildFid(OBJ_TYPE_TILE, fid, 0, 0, 0);
}

static void draw_scenery_art(int fid, int x, int y, fallout::Rect* rect) {
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

static void draw_custom_asset(CkFrm* frm, int screenX, int screenY, fallout::Rect* rect, int dir = 0) {
	if (frm->frames[dir].empty()) return;
    const CkFrmFrame& frame = frm->frames[dir][0];

    int offsetX = screenX + 16 + frame.offsetX - (frame.width / 2);
    int offsetY = screenY + 12 + frame.offsetY - frame.height;

    fallout::Rect artRect;
    artRect.left   = offsetX;
    artRect.top    = offsetY;
    artRect.right  = offsetX + frame.width - 1;
    artRect.bottom = offsetY + frame.height - 1;

    fallout::Rect intersection;
    if (fallout::rectIntersection(&artRect, rect, &intersection) == -1) return;

    unsigned char* src = const_cast<unsigned char*>(frame.pixels.data());
    src += frame.width * (intersection.top - offsetY) + (intersection.left - offsetX);

    int light = fallout::lightGetAmbientIntensity();
    fallout::_dark_trans_buf_to_buf(
        src,
        fallout::rectGetWidth(&intersection),
        fallout::rectGetHeight(&intersection),
        frame.width,
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

		if (tileInstance.isCustomAsset()) {
			CkFrm* frm = ck_assets_resolve(gAssetRegistry, tileInstance.assetKey);
			if (frm) draw_custom_asset(frm, screenX, screenY, rect);
			continue;
		}

        int fid = ck_rendering_build_tile_fid(tileInstance.engineFid);
        tileRenderFloorExternal(fid, screenX, screenY, rect);
    }
}

static void ck_rendering_scenery(fallout::Rect* rect) {
    for (const auto& scenery : gPersistentScenery) {
        int screenX, screenY;
        tileToScreenXY(scenery.tile, &screenX, &screenY);

		if (scenery.isCustomAsset()) {
			CkFrm* frm = ck_assets_resolve(gAssetRegistry, scenery.assetKey);
			if (frm) draw_custom_asset(frm, screenX, screenY, rect);
			continue;
		}

        int fid = ck_rendering_build_scenery_fid(scenery.engineFid);

        const CachedArt* cached = get_or_cache_art(fid);
        if (cached == nullptr || cached->frameData == nullptr) continue;

		int frameX = 0, frameY = 0;
		int rotationX = 0, rotationY = 0;

		fallout::artGetFrameOffsets(cached->art, 0, 0, &frameX, &frameY);
        fallout::artGetRotationOffsets(cached->art, 0, &rotationX, &rotationY);

		int screenCenterX = screenX + 16, screenCenterY = screenY + 12;

		int offsetX = screenCenterX + frameX + rotationX - (cached->width / 2);
		int offsetY = screenCenterY + frameY + rotationY - cached->height;

        draw_scenery_art(fid, offsetX, offsetY, rect);
    }
}
