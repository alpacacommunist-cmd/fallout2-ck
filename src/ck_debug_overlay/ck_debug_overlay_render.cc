#include "ck_debug_overlay/ck_debug_overlay_render.h"
#include "ck_debug_overlay/ck_debug_overlay_hexes.h"

#include <unordered_map>
#include <vector>
#include <cstring>

#include <object.h>
#include <tile.h>
#include <art.h>

struct HexBuffer {
    std::vector<unsigned char> data;
    int width = 0;
    int height = 0;

    bool empty() const { return data.empty(); }
};

static std::unordered_map<int, HexBuffer> gPrecoloredCache;

static const HexBuffer& get_or_create_colored_hex(unsigned char edgeColor, unsigned char innerColor) {
    int colorKey = (edgeColor << 8) | innerColor;

    // search cache
    auto it = gPrecoloredCache.find(colorKey);
    if (it != gPrecoloredCache.end()) {
        return it->second;
    }

    // new buffer if nothing found in cache
    HexBuffer newHex;
    int fid = ck_debug_overlay_build_interface_fid(CK_DEBUG_HEX_ART_ID);
    fallout::CacheEntry* cacheEntry = nullptr;
    fallout::Art* art = artLock(fid, &cacheEntry);

    if (art != nullptr) {
        newHex.width = artGetWidth(art, 0, 0);
        newHex.height = artGetHeight(art, 0, 0);
        int size = newHex.width * newHex.height;

        newHex.data.resize(size);
        const unsigned char* baseData = artGetFrameData(art, 0, 0);

        if (baseData != nullptr) {
            for (int i = 0; i < size; i++) {
                unsigned char pixel = baseData[i];
                if (pixel == 0) {
                    newHex.data[i] = 0; // Прозрачный
                } else if (pixel > 150) {
                    newHex.data[i] = edgeColor;
                } else {
                    newHex.data[i] = innerColor;
                }
            }
        }
        artUnlock(cacheEntry);
    }

    if (newHex.empty()) {
        static const HexBuffer emptyBuffer;
        return emptyBuffer;
    }

    return gPrecoloredCache.emplace(colorKey, std::move(newHex)).first->second;
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
     const HexBuffer& hex = get_or_create_colored_hex(edgeColor, innerColor);
     if (hex.empty()) return;

     int width = hex.width;
     int height = hex.height;

     fallout::Rect artRect;
     artRect.left = x, artRect.top = y, artRect.right = x + width - 1, artRect.bottom = y + height - 1;

     fallout::Rect intersection;
     if (fallout::rectIntersection(&artRect, rect, &intersection) == -1) return;

     const unsigned char* src = hex.data.data();
     src += width * (intersection.top - y) + (intersection.left - x);

     blit_debug_hex_colored(src, rectGetWidth(&intersection), rectGetHeight(&intersection),
              width, fallout::tileGetWindowBuffer(), intersection.left, intersection.top,
              fallout::tileGetWindowPitch());
}

int ck_debug_overlay_build_interface_fid(int artId) {
    return fallout::buildFid(fallout::OBJ_TYPE_INTERFACE, artId, 0, 0, 0);
}

void ck_debug_overlay_persistent_hexes(fallout::Rect* rect) {
    const auto& hexes = ck_debug_overlay_get_all_hexes();
    int fid = ck_debug_overlay_build_interface_fid(CK_DEBUG_HEX_ART_ID);

    for (const auto& [tile, hex] : hexes) {
        int screenX, screenY;
        fallout::tileToScreenXY(hex.tile, &screenX, &screenY);

        DebugHexColor color = (hex.state == HexState::CUSTOM)
                              ? hex.customColor
                              : ck_debug_get_color_for_state(hex.state);

        draw_misc_art(fid, screenX, screenY, rect, color.edge, color.inner);
    }
}

void ck_debug_overlay_render_clear() {
    ck_debug_overlay_clear_hexes();

	gPrecoloredCache.clear();

    fallout::tileWindowRefresh();
}
