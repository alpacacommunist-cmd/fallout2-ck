#include "ck_debug_overlay/ck_debug_overlay_render.h"
#include "ck_debug_overlay/ck_debug_overlay_hexes.h"

#include <cstdlib>
#include <cstring>

#include "color.h"
#include "tile.h"
#include "object.h"
#include "art.h"
#include "draw.h"

struct CachedHexArt {
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
};

struct PrecoloredHex {
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
};

static std::map<int, PrecoloredHex> gPrecoloredCache;
static CachedHexArt gBaseHexArt;

static PrecoloredHex get_or_create_colored_hex(unsigned char edgeColor, unsigned char innerColor) {
	// base art load
    if (gBaseHexArt.data == nullptr) {
        int fid = ck_debug_overlay_build_interface_fid(999);
        fallout::CacheEntry* cacheEntry;
        fallout::Art* art = artLock(fid, &cacheEntry);

        if (art != nullptr) {
            gBaseHexArt.width = artGetWidth(art, 0, 0);
			gBaseHexArt.height = artGetHeight(art, 0, 0);
            int size = gBaseHexArt.width * gBaseHexArt.height;

            gBaseHexArt.data = (unsigned char*)malloc(size);

            if (gBaseHexArt.data) memcpy(gBaseHexArt.data, artGetFrameData(art, 0, 0), size);
            artUnlock(cacheEntry);
        }
    }
    
    if (gBaseHexArt.data == nullptr) return PrecoloredHex{};

    // look for precolored hex
    int colorKey = (edgeColor << 8) | innerColor;
    auto it = gPrecoloredCache.find(colorKey);

	// return precolored hex if found in cache
    if (it != gPrecoloredCache.end()) return it->second;

    // generate newHex if precolored version is not cached yet
    PrecoloredHex newHex;
    newHex.width = gBaseHexArt.width;
    newHex.height = gBaseHexArt.height;
    int size = newHex.width * newHex.height;
    newHex.data = (unsigned char*)malloc(size);

    if (newHex.data) {
        // paint buffer
        for (int i = 0; i < size; i++) {
            unsigned char pixel = gBaseHexArt.data[i];
            if (pixel == 0) {
                newHex.data[i] = 0; // transparent
            } else if (pixel > 150) {
                newHex.data[i] = edgeColor;
            } else {
                newHex.data[i] = innerColor;
            }
        }
		// adds to cache
        gPrecoloredCache[colorKey] = newHex;
    }

    return newHex;
}

void blit_debug_hex_colored(const unsigned char* src, int width, int height, int srcPitch,
        unsigned char* dest, int destX, int destY, int destPitch) {
    for (int y = 0; y < height; y++) {
        const unsigned char* srcRow = src + (y * srcPitch);
        unsigned char* destRow = dest + ((destY + y) * destPitch) + destX;

        for (int x = 0; x < width; x++) {
            unsigned char pixel = srcRow[x];
            
            // branch prediction chill
            if (pixel == 0) continue; 

            destRow[x] = pixel;
        }
    }
}

static void draw_misc_art(int fid, int x, int y, fallout::Rect* rect,
        unsigned char edgeColor, unsigned char innerColor) {
    
    PrecoloredHex hex = get_or_create_colored_hex(edgeColor, innerColor);
    if (hex.data == nullptr) return;

    int width = hex.width;
    int height = hex.height;

    fallout::Rect artRect;
    artRect.left = x, artRect.top = y, artRect.right = x + width - 1, artRect.bottom = y + height - 1;

    fallout::Rect intersection;
    if (fallout::rectIntersection(&artRect, rect, &intersection) == -1) return;

    unsigned char* src = hex.data;
    src += width * (intersection.top - y) + (intersection.left - x);

    blit_debug_hex_colored(src, rectGetWidth(&intersection), rectGetHeight(&intersection),
            width, fallout::tileGetWindowBuffer(), intersection.left, intersection.top,
            fallout::tileGetWindowPitch());
}


int ck_debug_overlay_build_interface_fid(int artId) {
    return fallout::buildFid(fallout::OBJ_TYPE_INTERFACE, artId, 0, 0, 0);
}

void ck_debug_overlay_persistent_hexes(fallout::Rect* rect) {
    int anchorScreenX, anchorScreenY;
    const auto& hexes = ck_debug_overlay_get_all_hexes();

    for (const auto& [tile, hex] : hexes) {
        int fid = ck_debug_overlay_build_interface_fid(hex.artId);
        fallout::tileToScreenXY(hex.anchorTile, &anchorScreenX, &anchorScreenY);

        int screenX, screenY;
        fallout::tileToScreenXY(hex.tile, &screenX, &screenY);

        draw_misc_art(fid, screenX, screenY, rect, hex.color.edge, hex.color.inner);
    }
}

void ck_debug_overlay_clear() {
    ck_debug_overlay_clear_hexes();

    for (auto& [key, hex] : gPrecoloredCache) {
        if (hex.data != nullptr) std::free(hex.data);
    }
    gPrecoloredCache.clear();

    if (gBaseHexArt.data != nullptr) {
        std::free(gBaseHexArt.data);
        gBaseHexArt.data = nullptr;
    }

    fallout::tileWindowRefresh();
}
