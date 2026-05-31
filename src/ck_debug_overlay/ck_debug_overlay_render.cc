#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"

#include "color.h"
#include "tile.h"
#include "object.h"
#include "art.h"
#include "draw.h"

static void draw_misc_art(int fid, int x, int y, fallout::Rect* rect) {
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

	// default red af
    unsigned char edgeColor = 135;
    unsigned char innerColor = 135;

	// maps reserved (996 - 999) to debug colors
	// interface fids 996-999 are reserved for debug purposes
	// check art.cc 
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
			width, fallout::tileGetWindowBuffer(), intersection.left, intersection.top,
			fallout::tileGetWindowPitch(), edgeColor, innerColor
			);

	artUnlock(cacheEntry);
}

static std::vector<CkDebugHexInstance> gPersistentHexes;

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

	fallout::tileWindowRefresh();
}

void ck_debug_overlay_add_hex(int fid, int anchorTile, int offsetX, int offsetY) {
    gPersistentHexes.push_back({ fid, anchorTile, offsetX, offsetY });
}

int ck_debug_overlay_build_interface_fid(int fid) {
	return fallout::buildFid(fallout::OBJ_TYPE_INTERFACE, fid, 0, 0, 0);
}

void ck_debug_overlay_persistent_hexes(fallout::Rect* rect) {
	for (const auto& hex : gPersistentHexes) {
		int screenX, screenY;
		fallout::tileToScreenXY(hex.anchorTile, &screenX, &screenY);

		int fid = ck_debug_overlay_build_interface_fid(hex.fid);

		draw_misc_art(fid, screenX + hex.offsetX, screenY + hex.offsetY, rect);
	}
}

