#include <vector>

#include "art.h"
#include "cache.h"
#include "debug.h"
#include "geometry.h"
#include "map.h"
#include "light.h"
#include "mouse.h"
#include "object.h"
#include "draw.h"
#include "tile.h"

#include "ck_rendering.h"

// frame queue
struct CkSceneryDrawRequest { int fid; int x; int y; };
static std::vector<CkSceneryDrawRequest> gSceneryDrawRequests;

// persistent queues
struct CkSceneryInstance {int fid; int tile; int offsetX; int offsetY;};
static std::vector<CkSceneryInstance> gPersistentScenery;

struct CkTileInstance {int fid; int tile; int offsetX; int offsetY; };
static std::vector<CkTileInstance> gPersistentTiles;

struct CkMiscInstance {int fid; int tile; int offsetX; int offsetY; };
static std::vector<CkMiscInstance> gPersistentMisc;

// frame
void ck_rendering_draw_scenery(int fid, int x, int y) {
    gSceneryDrawRequests.push_back({ fid, x, y });
}


// persistent
void ck_rendering_add_scenery(int fid, int tile, int offsetX, int offsetY) {
    gPersistentScenery.push_back({ fid, tile, offsetX, offsetY });
}

void ck_rendering_add_tile(int fid, int tile, int offsetX, int offsetY) {
    gPersistentTiles.push_back({ fid, tile, offsetX, offsetY });
}

void ck_rendering_add_misc(int fid, int tile, int offsetX, int offsetY) {
    gPersistentMisc.push_back({ fid, tile, offsetX, offsetY });
}

void ck_rendering_clear() {
	gPersistentScenery.clear();
	gPersistentTiles.clear();
	gPersistentMisc.clear();

	ck_rendering_clear_camera_borders();
}

using namespace fallout;

// frame queue
static void ck_rendering_draw(fallout::Rect* rect);
// persistent queue
static void ck_rendering_scenery(fallout::Rect* rect);
static void ck_rendering_tiles(fallout::Rect* rect);
static void ck_rendering_misc(fallout::Rect* rect);

static CkCameraBorders gCameraBorders;

void ck_rendering_render(fallout::Rect* rect) {
	ck_rendering_scenery(rect);
	ck_rendering_tiles(rect);
	ck_rendering_misc(rect);

	ck_rendering_draw(rect);
}

bool ck_rendering_is_camera_position_allowed(int tile) {
    if (!gCameraBorders.enabled) {
        return false;
    }

    int gridWidth = fallout::tileGetHexGridWidth();
    int tileX = gridWidth - 1 - tile % gridWidth;
    int tileY = tile / gridWidth;

    bool allowed = (tileX >= gCameraBorders.left && tileX <= gCameraBorders.right) &&
        (tileY >= gCameraBorders.top && tileY <= gCameraBorders.bottom);

    // fallout::debugPrint("[CK] Camera check " "tile=(%d,%d) " "bounds=(%d..%d,%d..%d) " "allowed=%d\n",
    //     tileX, tileY, gCameraBorders.left, gCameraBorders.right, gCameraBorders.top, gCameraBorders.bottom, allowed);

	return allowed;
}

bool ck_rendering_has_camera_borders() { return gCameraBorders.enabled; }

int ck_rendering_build_scenery_fid(int fid) {
    return buildFid(OBJ_TYPE_SCENERY, fid, 0, 0, 0);
}

int ck_rendering_build_tile_fid(int fid) {
    return buildFid(OBJ_TYPE_TILE, fid, 0, 0, 0);
}

int ck_rendering_build_interface_fid(int fid) {
    return buildFid(OBJ_TYPE_INTERFACE, fid, 0, 0, 0);
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

static void blit_debug_hex_colored(
    const unsigned char* src, int width, int height, int srcPitch,
    unsigned char* dest, int destX, int destY, int destPitch,
    unsigned char edgeColor, unsigned char innerColor
) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char pixel = src[y * srcPitch + x];

            // 0x00 - прозрачный фон, его всегда пропускаем
            if (pixel == 0) continue;

            int outX = destX + x;
            int outY = destY + y;
            unsigned char* destPixel = dest + (outY * destPitch) + outX;

            // Если оригинальный пиксель достаточно яркий (граница хекса),
            // красим его основным ярким цветом
            if (pixel > 150) {
                *destPixel = edgeColor;
            } else {
                // Если это внутренняя полупрозрачная тень,
                // заливаем её приглушенным оттенком того же цвета для объема
                *destPixel = innerColor;
                
                // АЛЬТЕРНАТИВА: Если ты хочешь, чтобы хекс внутри был 100% ПУСТЫМ,
                // просто закомментируй строчку выше (*destPixel = innerColor;).
            }
        }
    }
}

static void draw_misc_art(int fid, int x, int y, Rect* rect) {
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

    // Сдвигаем указатель источника с учетом пересечения
    src += width * (intersection.top - y) + (intersection.left - x);

    // Вытаскиваем чистый ID из FID, чтобы узнать, какой цвет от нас хотят
    int artId = fid & 0xFFFF;

	   
    // defalut red
    unsigned char edgeColor = 135;
    unsigned char innerColor = 135;
    
	if (artId == 998) { 
		edgeColor = 198; // green
		innerColor = 198;
	} 
	else if (artId == 997) { 
		edgeColor = 57; // yellow
		innerColor = 57;
	} 
	else if (artId == 996) { 
		edgeColor = 105; //
		innerColor = 105; // blue
	}

	blit_debug_hex_colored(src, rectGetWidth(&intersection), rectGetHeight(&intersection),
			width, tileGetWindowBuffer(), intersection.left, intersection.top,
			tileGetWindowPitch(), edgeColor, innerColor
	);

    artUnlock(cacheEntry);
}

static void ck_rendering_tiles(fallout::Rect* rect) {
    for (const auto& tileInstance : gPersistentTiles) {

        int screenX;
        int screenY;

        tileToScreenXY(tileInstance.tile, &screenX, &screenY);
        int fid = ck_rendering_build_tile_fid(tileInstance.fid);

        tileRenderFloorExternal(fid, screenX + tileInstance.offsetX, screenY + tileInstance.offsetY, rect);
    }
}

static void ck_rendering_scenery(fallout::Rect* rect) {
    for (const auto& scenery : gPersistentScenery) {
        int screenX;
        int screenY;

        tileToScreenXY(scenery.tile, &screenX, &screenY);

        int fid = ck_rendering_build_scenery_fid(scenery.fid);

        draw_scenery_art(fid, screenX + scenery.offsetX, screenY + scenery.offsetY, rect);
    }
}

static void ck_rendering_misc(fallout::Rect* rect) {
	for (const auto& scenery : gPersistentMisc) {
		int screenX, screenY;
		tileToScreenXY(scenery.tile, &screenX, &screenY);

		int fid = ck_rendering_build_interface_fid(scenery.fid);

		draw_misc_art(fid, screenX + scenery.offsetX, screenY + scenery.offsetY, rect);
	}
}


void ck_rendering_set_camera_borders(int left, int right, int top, int bottom) {
    gCameraBorders.enabled = true;

    gCameraBorders.left = left;
    gCameraBorders.right = right;
    gCameraBorders.top = top;
    gCameraBorders.bottom = bottom;
}

void ck_rendering_clear_camera_borders() { gCameraBorders = {}; }

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

