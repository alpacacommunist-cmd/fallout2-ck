#include "ck_rendering.h"

#include "art.h"
#include "cache.h"
#include "debug.h"
#include "geometry.h"
#include "map.h"
#include "mouse.h"
#include "object.h"
#include "draw.h"
#include "tile.h"

using namespace fallout;

static void drawSceneryArt(int fid, int x, int y, Rect* rect)
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

    blitBufferToBufferTrans(
        src,
        rectGetWidth(&intersection),
        rectGetHeight(&intersection),
        width,
        tileGetWindowBuffer()
            + tileGetWindowPitch() * intersection.top
            + intersection.left,
        tileGetWindowPitch());

    artUnlock(cacheEntry);
}

static void drawMountainPatch(int anchorScreenX, int anchorScreenY, Rect* rect) {
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

void drawTestOutskirts(Rect* rect)
{
    if (mapGetCurrentMap() != 126) {
        return;
    }

    int anchorTile = 17290;
    int screenX;
    int screenY;

    tileToScreenXY(
        anchorTile,
        &screenX,
        &screenY);


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

    drawSceneryArt(fid1, screenX - 500, screenY - 360, rect);
    drawSceneryArt(fid2, screenX - 725, screenY - 360, rect);
    drawSceneryArt(fid3, screenX - 725, screenY - 230, rect);
    drawSceneryArt(fid4, screenX - 492, screenY - 250, rect);
    drawSceneryArt(fid5, screenX - 693, screenY - 55, rect);
    drawSceneryArt(fid6, screenX - 465, screenY - 155, rect);
    drawSceneryArt(fid7, screenX - 690, screenY - 160, rect);
    drawSceneryArt(fid8, screenX - 500, screenY - 80, rect);
    // drawSceneryArt(fid9, screenX - 670, screenY - 60, rect);

    drawMountainPatch(screenX, screenY, rect);
    // right cliff
    int fid9 = buildFid(OBJ_TYPE_SCENERY, 702, 0, 0, 0);
    drawSceneryArt(fid9, screenX + 430, screenY - 390, rect);
    drawSceneryArt(fid8, screenX + 630, screenY - 415, rect);
}


void ck_rendering_draw(fallout::Rect* rect)
{
	drawTestOutskirts(rect);
}
