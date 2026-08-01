#include <unordered_map>
#include <algorithm>

#include "art.h"
#include "light.h"
#include "tile.h"

#include "ck_rendering.h"
#include "ck_assets/ck_asset_registry.h"

extern CkAssetRegistry gAssetRegistry;

std::vector<CkSceneryInstance> gPersistentScenery;
std::vector<CkTileInstance> gPersistentTiles;

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

static void blit_sub_buffer(const unsigned char* pixels, int src_width, int dest_x, int dest_y, int width, int height, fallout::Rect* screen_rect) {
    fallout::Rect artRect;
    artRect.left   = dest_x;
    artRect.top    = dest_y;
    artRect.right  = dest_x + width - 1;
    artRect.bottom = dest_y + height - 1;

    fallout::Rect intersection;
    // out of screen
    if (fallout::rectIntersection(&artRect, screen_rect, &intersection) == -1) return;

    // src buf offset
    const unsigned char* src = pixels + src_width * (intersection.top - dest_y) + (intersection.left - dest_x);

    int light = fallout::lightGetAmbientIntensity();

    fallout::_dark_trans_buf_to_buf(
            const_cast<unsigned char*>(src),
            fallout::rectGetWidth(&intersection), fallout::rectGetHeight(&intersection), src_width,
            fallout::tileGetWindowBuffer(),
            intersection.left, intersection.top,
            fallout::tileGetWindowPitch(),
            light
    );
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
    CkSceneryInstance instance;
    instance.tile      = tile;
    instance.engineFid = fid;

	auto it = std::upper_bound(gPersistentScenery.begin(), gPersistentScenery.end(), instance,
	[](const CkSceneryInstance& a, const CkSceneryInstance& b) {
		return a.tile < b.tile;
	});
    gPersistentScenery.insert(it, instance);
}

void ck_rendering_add_custom_scenery(const std::string& key, int tile) {
    CkSceneryInstance instance;
    instance.tile     = tile;
    instance.assetKey = key;

	auto it = std::upper_bound(gPersistentScenery.begin(), gPersistentScenery.end(), instance,
	[](const CkSceneryInstance& a, const CkSceneryInstance& b) {
		return a.tile < b.tile;
	});
    gPersistentScenery.insert(it, instance);
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

	gPersistentScenery.shrink_to_fit();
	gPersistentTiles.shrink_to_fit();

	ck_rendering_clear_art_cache();
}

void ck_rendering_refresh() {
	fallout::tileWindowRefresh();
}

using namespace fallout;

static void ck_rendering_scenery(fallout::Rect* rect);
static void ck_rendering_tiles(fallout::Rect* rect);

void ck_rendering_render(fallout::Rect* rect) {
	ck_rendering_tiles(rect);
	ck_rendering_scenery(rect);
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

    blit_sub_buffer(cached->frameData, cached->width, x, y, cached->width, cached->height, rect);
}

static void draw_custom_asset(CkFrm* frm, int screenX, int screenY, fallout::Rect* rect, int dir = 0) {
    if (frm->frames[dir].empty()) return;
    const CkFrmFrame& frame = frm->frames[dir][0];

    int offsetX = screenX + 16 + frame.offsetX - (frame.width / 2);
    int offsetY = screenY + 12 + frame.offsetY - frame.height;

    blit_sub_buffer(frame.pixels.data(), frame.width, offsetX, offsetY, frame.width, frame.height, rect);
}

static void ck_rendering_tiles(fallout::Rect* rect) {
    for (const auto& tileInstance : gPersistentTiles) {
        int screenX, screenY;
        tileToScreenXY(tileInstance.tile, &screenX, &screenY);

		if (tileInstance.isCustomAsset()) {
			CkFrm* frm = ck_assets_resolve_frm(gAssetRegistry, tileInstance.assetKey);
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
			CkFrm* frm = ck_assets_resolve_frm(gAssetRegistry, scenery.assetKey);
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

