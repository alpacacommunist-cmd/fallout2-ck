#include <unordered_map>
#include <algorithm>
#include <chrono>

#include "art.h"
#include "light.h"
#include "tile.h"

#include "ck_rendering.h"

#include "ck_log.h"
static const Logger log("CK Rendering");

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

static inline bool is_tile_visible(int screenX, int screenY, const fallout::Rect* rect, int paddingX, int paddingY) {
    return screenX >= (rect->left - paddingX) && screenX <= (rect->right + paddingX) &&
           screenY >= (rect->top - paddingY) && screenY <= (rect->bottom + paddingY);
}

void ck_rendering_clear_art_cache() {
    for (auto &[fid, cached] : gArtCache) {
        if (cached.cacheEntry) {
            fallout::artUnlock(cached.cacheEntry);
        }
    }
    gArtCache.clear();
}

void ck_rendering_add_scenery(int fid, int tile) {
    CkSceneryInstance instance;
    instance.tile = tile;
    instance.fid  = fid;

	auto it = std::upper_bound(gPersistentScenery.begin(), gPersistentScenery.end(), instance,
	[](const CkSceneryInstance& a, const CkSceneryInstance& b) {
		return a.tile < b.tile;
	});
    gPersistentScenery.insert(it, instance);
}

void ck_rendering_add_tile(int fid, int tile) {
    CkTileInstance inst;
    inst.tile      = tile;
    inst.fid       = fid;
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


static void draw_scenery_art(int fid, int x, int y, fallout::Rect* rect) {
    const CachedArt* cached = get_or_cache_art(fid);
    if (cached == nullptr || cached->frameData == nullptr) return;

    blit_sub_buffer(cached->frameData, cached->width, x, y, cached->width, cached->height, rect);
}

static int ck_rendering_tiles(fallout::Rect* rect) {
    const int TILE_PADDING_X = 80;
    const int TILE_PADDING_Y = 40;
    int visible_count = 0;

    for (const auto& tile_instance : gPersistentTiles) {
        int screenX, screenY;
        tileToScreenXY(tile_instance.tile, &screenX, &screenY);

        if (!is_tile_visible(screenX, screenY, &fallout::tileWindowRect(), TILE_PADDING_X, TILE_PADDING_Y)) {
            continue;
        }

        visible_count++;

        tileRenderFloorExternal(tile_instance.fid, screenX, screenY, rect);
    }

    return visible_count;
}

static int ck_rendering_scenery(fallout::Rect* rect) {
    const int SCENERY_PADDING_X = 160;
    const int SCENERY_PADDING_Y = 240;
    int visible_count = 0;

    for (const auto& scenery : gPersistentScenery) {
        int screenX, screenY;
        tileToScreenXY(scenery.tile, &screenX, &screenY);

        if (!is_tile_visible(screenX, screenY, &fallout::tileWindowRect(), SCENERY_PADDING_X, SCENERY_PADDING_Y)) {
            continue;
        }

        visible_count++;

        const CachedArt* cached = get_or_cache_art(scenery.fid);
        if (cached == nullptr || cached->frameData == nullptr) continue;

        int frameX = 0, frameY = 0;
        int rotationX = 0, rotationY = 0;

        fallout::artGetFrameOffsets(cached->art, 0, 0, &frameX, &frameY);
        fallout::artGetRotationOffsets(cached->art, 0, &rotationX, &rotationY);

        int screenCenterX = screenX + 16, screenCenterY = screenY + 12;

        int offsetX = screenCenterX + frameX + rotationX - (cached->width / 2);
        int offsetY = screenCenterY + frameY + rotationY - cached->height;

        draw_scenery_art(scenery.fid, offsetX, offsetY, rect);
    }

    return visible_count;
}

void ck_rendering_render(fallout::Rect* rect) {
	int visible_tiles   = ck_rendering_tiles(rect);
	int visible_scenery = ck_rendering_scenery(rect);

    static auto last_log_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    if (now - last_log_time >= std::chrono::seconds(2)) {
        last_log_time = now;

        log.debug("Culling info: Tiles: {}/{} | Scenery: {}/{}",
                visible_tiles, gPersistentTiles.size(),
                visible_scenery, gPersistentScenery.size());
    }
}

