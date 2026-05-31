#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"

#include "color.h"
#include "tile.h"
#include "object.h"
#include "art.h"
#include "draw.h"

static void draw_test_hex(int fid, int x, int y, fallout::Rect* rect, int edgeColor, int innerColor) {
	fallout::CacheEntry* cacheEntry;
	fallout::Art* art = artLock(fid, &cacheEntry);

    if (art == nullptr) return;

    int width = artGetWidth(art, 0, 0);
    int height = artGetHeight(art, 0, 0);

	fallout::Rect artRect;
    artRect.left = x, artRect.top = y, artRect.right = x + width - 1, artRect.bottom = y + height - 1;

	fallout::Rect intersection;
    if (fallout::rectIntersection(&artRect, rect, &intersection) == -1) {
		fallout::artUnlock(cacheEntry);
        return;
    }

    unsigned char* src = fallout::artGetFrameData(art, 0, 0);

    src += width * (intersection.top - y) + (intersection.left - x);

    int artId = fid & 0xFFFF;
	   
	blit_debug_hex_colored(src, rectGetWidth(&intersection), rectGetHeight(&intersection),
			width, fallout::tileGetWindowBuffer(), intersection.left, 
			intersection.top, fallout::tileGetWindowPitch(), edgeColor, innerColor
	);

    artUnlock(cacheEntry);
}

void ck_debug_overlay_palette_hex_test(fallout::Rect* rect) {
    const int anchorTile = 17290;

    int baseX;
    int baseY;
    fallout::tileToScreenXY(anchorTile, &baseX, &baseY);

    int fid = ck_debug_overlay_build_interface_fid(999);

    for (int color = 0; color < 256; color++) {

        int col = color % 16;
        int row = color / 16;

        draw_test_hex(fid, baseX + col * 36, baseY + row * 20, rect, color, color);
		// ck_debug_overlay_add_hex(999, anchorTile, screenX - anchorScreenX, screenY - anchorScreenY, color, color);
    }
}

