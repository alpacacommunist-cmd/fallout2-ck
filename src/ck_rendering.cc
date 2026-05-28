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

void ck_rendering_clear() {
	gPersistentScenery.clear();
	gPersistentTiles.clear();
}

using namespace fallout;

static void ck_rendering_draw(fallout::Rect* rect);
static void ck_rendering_add(fallout::Rect* rect);
static void ck_rendering_tiles(fallout::Rect* rect);

void ck_rendering_render(fallout::Rect* rect) {
	ck_rendering_tiles(rect);
	ck_rendering_add(rect);
	ck_rendering_draw(rect);
}

static int build_scenery_fid(int fid) {
    return buildFid(OBJ_TYPE_SCENERY, fid, 0, 0, 0);
}

static int build_tile_fid(int fid) {
    return buildFid(OBJ_TYPE_TILE, fid, 0, 0, 0);
}

static void ck_rendering_tiles(fallout::Rect* rect) {
    for (const auto& tileInstance : gPersistentTiles) {

        int screenX;
        int screenY;

        tileToScreenXY(tileInstance.tile, &screenX, &screenY);
        int fid = build_tile_fid(tileInstance.fid);

        tileRenderFloorExternal(fid, screenX + tileInstance.offsetX, screenY + tileInstance.offsetY, rect);
    }
}



// refactor zone




static void draw_scenery_art(int fid, int x, int y, Rect* rect)
{
    CacheEntry* cacheEntry;
    Art* art = artLock(fid, &cacheEntry);
    if (art == nullptr) {
        return;
    }

    int width = artGetWidth(art, 0, 0);
    int height = artGetHeight(art, 0, 0);

    Rect artRect;
    artRect.left = x;
    artRect.top = y;
    artRect.right = x + width - 1;
    artRect.bottom = y + height - 1;

    Rect intersection;
    if (rectIntersection(
            &artRect,
            rect,
            &intersection) == -1) {
        artUnlock(cacheEntry);
        return;
    }

    unsigned char* src =
        artGetFrameData(art, 0, 0);

    src +=
        width * (intersection.top - y)
        + (intersection.left - x);

	int light = lightGetAmbientIntensity();
	_dark_trans_buf_to_buf(src, rectGetWidth(&intersection), rectGetHeight(&intersection),
			width, tileGetWindowBuffer(), intersection.left, intersection.top,
			tileGetWindowPitch(), light);

    artUnlock(cacheEntry);
}

static void draw_mountain_patch(int anchorScreenX, int anchorScreenY, Rect* rect) {
    struct BrushTile {
        int fidIndex;
        int dx;
        int dy;
    };

    static const BrushTile patch[] = {
        // === triangle near cursor ===
        {1505, -322, -84},
        {1506, -334, -79},
        {1507, -312, -74},
        {1508, -326, -68},

        {1505, -322, -124},
        {1506, -334, -119},
        {1507, -312, -114},
        {1508, -326, -108},

        // slightly below
        {1505, -336, -58},
        {1509, -318, -58},

        // === left big gap ===
        {1505, -530, -72},
        {1506, -562, -70},
        {1507, -595, -78},
        {1508, -635, -74},
        {1507, -685, -62},

        {1505, -530, -52},
        {1506, -562, -50},
        {1507, -595, -58},
        {1508, -635, -54},
        {1507, -685, -42},

        {1505, -385, -167},


        // {1505, -321, -77},
        // {1505, -334, -41},
        // {1506, -324, -79},
        // {1507, -326, -80},
    };

    for (const auto& tile : patch) {
        int fid = buildFid(OBJ_TYPE_TILE, tile.fidIndex, 0, 0, 0);

        tileRenderFloorExternal(fid, anchorScreenX + tile.dx, anchorScreenY + tile.dy, rect);
    }
}

void draw_test_outskirts(Rect* rect)
{
    if (mapGetCurrentMap() != 126) {
        return;
    }

    int anchorTile = 17290;
    int screenX;
    int screenY;

    tileToScreenXY(anchorTile, &screenX, &screenY);


    static int lastMouseX = -9999;
    static int lastMouseY = -9999;

    int mouseX;
    int mouseY;
    mouseGetPosition(&mouseX, &mouseY);

    if (mouseX != lastMouseX || mouseY != lastMouseY)
    {
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        int relativeX =
            mouseX - screenX;

        int relativeY =
            mouseY - screenY;

        if (mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) {
            debugPrint("{1505, %+d, %+d},\n",
                    relativeX,
                    relativeY);
        }
    }

    // left cliff
    // ⬅️⬅️
    // ⬇️➡️
    int fid1 = buildFid(OBJ_TYPE_SCENERY, 704, 0, 0, 0);
    int fid2 = buildFid(OBJ_TYPE_SCENERY, 703, 0, 0, 0);
    int fid3 = buildFid(OBJ_TYPE_SCENERY, 702, 0, 0, 0);
    // int fid4 = buildFid(OBJ_TYPE_SCENERY, 705, 0, 0, 0);
    int fid4 = buildFid(OBJ_TYPE_SCENERY, 1384, 0, 0, 0);
    int fid5 = buildFid(OBJ_TYPE_SCENERY, 702, 0, 0, 0);
    int fid6 = buildFid(OBJ_TYPE_SCENERY, 701, 0, 0, 0);
    int fid7 = buildFid(OBJ_TYPE_SCENERY, 1384, 0, 0, 0);
    int fid8 = buildFid(OBJ_TYPE_SCENERY, 706, 0, 0, 0);
    // int fid9 = buildFid(OBJ_TYPE_SCENERY, 701, 0, 0, 0);

    draw_scenery_art(fid1, screenX - 500, screenY - 360, rect);
    draw_scenery_art(fid2, screenX - 725, screenY - 360, rect);
    draw_scenery_art(fid3, screenX - 725, screenY - 230, rect);
    draw_scenery_art(fid4, screenX - 492, screenY - 250, rect);
    draw_scenery_art(fid5, screenX - 693, screenY - 55, rect);
    draw_scenery_art(fid6, screenX - 465, screenY - 155, rect);
    draw_scenery_art(fid7, screenX - 690, screenY - 160, rect);
    draw_scenery_art(fid8, screenX - 500, screenY - 80, rect);
    // drawSceneryArt(fid9, screenX - 670, screenY - 60, rect);

    draw_mountain_patch(screenX, screenY, rect);
    // right cliff
    int fid9 = buildFid(OBJ_TYPE_SCENERY, 702, 0, 0, 0);
    draw_scenery_art(fid9, screenX + 430, screenY - 390, rect);
    draw_scenery_art(fid8, screenX + 630, screenY - 415, rect);
}


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

	// draw_test_outskirts(rect);
}

static void ck_rendering_add(fallout::Rect* rect) {
    for (const auto& scenery : gPersistentScenery) {
        int screenX;
        int screenY;

        tileToScreenXY(scenery.tile, &screenX, &screenY);

        int fid = build_scenery_fid(scenery.fid);

        draw_scenery_art(fid, screenX + scenery.offsetX, screenY + scenery.offsetY, rect);
    }
}
