#include <unordered_map>
#include <algorithm>
#include <chrono>

#include "art.h"
#include "light.h"
#include "tile.h"

#include "ck_rendering.h"

#include "ck_log.h"
static const Logger logger("CK Rendering");

std::vector<CkSceneryInstance> gScenery;
std::vector<CkTileInstance> gTiles;

std::vector<CkSceneryInstance> gRoofScenery;
std::vector<CkTileInstance> gRoofTiles;

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
    cached.width = fallout::artGetWidth(art, 0, static_cast<fallout::Rotation>(0));
    cached.height = fallout::artGetHeight(art, 0, static_cast<fallout::Rotation>(0));
    cached.frameData = fallout::artGetFrameData(art, 0, static_cast<fallout::Rotation>(0));

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
    // ck::assets::clear();
}

void ck_rendering_add_scenery(int fid, int tile, CkRenderLayer layer, int offset_y) {
    CkSceneryInstance instance;
    instance.tile = tile;
    instance.offset_y = offset_y;
    instance.fid  = fid;

    auto& target_vector = (layer == CkRenderLayer::Roof) ? gRoofScenery : gScenery;

    auto it = std::upper_bound(target_vector.begin(), target_vector.end(), instance,
            [](const CkSceneryInstance& a, const CkSceneryInstance& b) {
                return a.tile < b.tile;
            });

    target_vector.insert(it, instance);
}

void ck_rendering_add_tile(int fid, int tile, CkRenderLayer layer) {
    CkTileInstance inst;
    inst.tile = tile;
    inst.fid = fid;

    auto &target_vector = (layer == CkRenderLayer::Roof) ? gRoofTiles : gTiles;

    target_vector.push_back(inst);
}

void ck_rendering_clear() {
    auto clear_vector = [](auto& vec) {
        vec.clear();
        vec.shrink_to_fit();
    };

    clear_vector(gScenery); clear_vector(gTiles);
    clear_vector(gRoofScenery); clear_vector(gRoofTiles);

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

static int ck_rendering_tiles(fallout::Rect* rect, const std::vector<CkTileInstance>& tiles, CkRenderLayer layer) {
    const int TILE_PADDING_X = 80;
    const int TILE_PADDING_Y = 40;
    int visible_count = 0;

    for (const auto& tile_instance : tiles) {
        int screenX, screenY;
        tileToScreenXY(tile_instance.tile, &screenX, &screenY);

        if (layer == CkRenderLayer::Roof) {
            screenY -= 96;
        }

        if (!is_tile_visible(screenX, screenY, &fallout::tileWindowRect(), TILE_PADDING_X, TILE_PADDING_Y)) {
            continue;
        }

        visible_count++;

        fallout::tileRenderRoofExternal(tile_instance.fid, screenX, screenY, rect);
    }

    return visible_count;
}

static int ck_rendering_scenery(fallout::Rect* rect, const std::vector<CkSceneryInstance>& scenery_list, CkRenderLayer layer) {
    const int SCENERY_PADDING_X = 160;
    const int SCENERY_PADDING_Y = 240;
    int visible_count = 0;

    for (const auto& scenery : scenery_list) {
        int screenX, screenY;
        tileToScreenXY(scenery.tile, &screenX, &screenY);

        if (layer == CkRenderLayer::Roof) {
            screenY += scenery.offset_y;
        }

        if (!is_tile_visible(screenX, screenY, &fallout::tileWindowRect(), SCENERY_PADDING_X, SCENERY_PADDING_Y)) {
            continue;
        }

        visible_count++;

        const CachedArt* cached = get_or_cache_art(scenery.fid);
        if (cached == nullptr || cached->frameData == nullptr) continue;

        int frameX = 0, frameY = 0;
        int rotationX = 0, rotationY = 0;

        fallout::artGetFrameOffsets(cached->art, 0, static_cast<fallout::Rotation>(0), &frameX, &frameY);
        fallout::artGetRotationOffsets(cached->art, static_cast<fallout::Rotation>(0), &rotationX, &rotationY);

        int screenCenterX = screenX + 16, screenCenterY = screenY + 12;

        int offsetX = screenCenterX + frameX + rotationX - (cached->width / 2);
        int offsetY = screenCenterY + frameY + rotationY - cached->height;

        draw_scenery_art(scenery.fid, offsetX, offsetY, rect);
    }

    return visible_count;
}

namespace ck::rendering {
    void floor(fallout::Rect* rect) {
        int visible_tiles   = ck_rendering_tiles(rect, gTiles, CkRenderLayer::Floor);
        int visible_scenery = ck_rendering_scenery(rect, gScenery, CkRenderLayer::Floor);

        static auto last_log_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();

        if (now - last_log_time >= std::chrono::seconds(10)) {
            last_log_time = now;

            logger.debug("Culling info [floor]: Tiles: {}/{} | Scenery: {}/{}",
                    visible_tiles, gTiles.size(),
                    visible_scenery, gScenery.size());
        }
    }

    void roof(fallout::Rect* rect) {
        int visible_tiles   = ck_rendering_tiles(rect, gRoofTiles, CkRenderLayer::Roof);
        int visible_scenery = ck_rendering_scenery(rect, gRoofScenery, CkRenderLayer::Roof);

        static auto last_log_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();

        if (now - last_log_time >= std::chrono::seconds(10)) {
            last_log_time = now;

            logger.debug("Culling info [roof]: Tiles: {}/{} | Scenery: {}/{}",
                    visible_tiles, gTiles.size(),
                    visible_scenery, gScenery.size());
        }
    }
}

