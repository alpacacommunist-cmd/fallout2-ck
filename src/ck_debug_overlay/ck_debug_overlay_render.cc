#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"

#include <cstdlib>
#include <cstring>
#include <map>

#include "color.h"
#include "tile.h"
#include "object.h"
#include "art.h"
#include "draw.h"

static std::map<int, CkDebugHex> gPersistentHexes;

struct CachedHexArt {
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
};

static CachedHexArt gCachedHex;

static void ck_debug_overlay_init_hex_cache() {
    if (gCachedHex.data != nullptr) return;

    int fid = ck_debug_overlay_build_interface_fid(999);

    fallout::CacheEntry* cacheEntry;
    fallout::Art* art = artLock(fid, &cacheEntry);

    if (art == nullptr) return;

    gCachedHex.width = artGetWidth(art, 0, 0);
    gCachedHex.height = artGetHeight(art, 0, 0);

    int size = gCachedHex.width * gCachedHex.height;

    gCachedHex.data = (unsigned char*)malloc(size);

    if (gCachedHex.data != nullptr)
        memcpy(gCachedHex.data, artGetFrameData(art, 0, 0), size);

    artUnlock(cacheEntry);
}

static void draw_misc_art(int fid, int x, int y, fallout::Rect* rect,
		unsigned char edgeColor, unsigned char innerColor) {
	ck_debug_overlay_init_hex_cache();
	if (gCachedHex.data == nullptr) return;

	int width = gCachedHex.width;
	int height = gCachedHex.height;

	fallout::Rect artRect;
	artRect.left = x, artRect.top = y, artRect.right = x + width - 1, artRect.bottom = y + height - 1;

	fallout::Rect intersection;
	if (fallout::rectIntersection(&artRect, rect, &intersection) == -1) return;

	unsigned char* src = gCachedHex.data;

	src += width * (intersection.top - y) + (intersection.left - x);

	blit_debug_hex_colored(src, rectGetWidth(&intersection), rectGetHeight(&intersection),
			width, fallout::tileGetWindowBuffer(), intersection.left, intersection.top,
			fallout::tileGetWindowPitch(), edgeColor, innerColor
			);
}

void blit_debug_hex_colored(
		const unsigned char* src, int width, int height, int srcPitch,
		unsigned char* dest, int destX, int destY, int destPitch,
		unsigned char edgeColor, unsigned char innerColor
		) {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			unsigned char pixel = src[y * srcPitch + x];

			// skip transparent
			if (pixel == 0) continue;

			int outX = destX + x;
			int outY = destY + y;
			unsigned char* destPixel = dest + (outY * destPitch) + outX;

			// > 150 is supposed to be bright, TODO: figure out something smarter
			if (pixel > 150) {
				*destPixel = edgeColor;
			} else {
				// adds thickness, comment out to make hex hollow
				*destPixel = innerColor;
			}
		}
	}
}

void ck_debug_overlay_clear() {
	gPersistentHexes.clear();

    free(gCachedHex.data);
    gCachedHex.data = nullptr;

	fallout::tileWindowRefresh();
}

void ck_debug_overlay_add_hex(int artId, int anchorTile, int tile, DebugHexColor color) {
	gPersistentHexes[tile] = { artId, anchorTile, tile, color };
}

void ck_debug_overlay_remove_hex(int tile) {
    gPersistentHexes.erase(tile);
}

int ck_debug_overlay_build_interface_fid(int artId) {
	return fallout::buildFid(fallout::OBJ_TYPE_INTERFACE, artId, 0, 0, 0);
}


CkDebugHex* ck_debug_overlay_find_hex(int tile) {
    auto it = gPersistentHexes.find(tile);

    if (it != gPersistentHexes.end()) return &(it->second);
    return nullptr;
}

void ck_debug_overlay_persistent_hexes(fallout::Rect* rect) {
    int anchorScreenX, anchorScreenY;

    for (const auto& [tile, hex] : gPersistentHexes) {
        int fid = ck_debug_overlay_build_interface_fid(hex.artId);

        fallout::tileToScreenXY(hex.anchorTile, &anchorScreenX, &anchorScreenY);
        int screenX, screenY;
        fallout::tileToScreenXY(hex.tile, &screenX, &screenY);


        draw_misc_art(fid, screenX, screenY, rect, hex.color.edge, hex.color.inner);
    }
}

