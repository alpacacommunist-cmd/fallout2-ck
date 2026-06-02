#ifndef CK_DEBUG_OVERLAY_RENDER_H
#define CK_DEBUG_OVERLAY_RENDER_H

#include <map>
#include <vector>

namespace fallout {
    struct Rect;
}

struct DebugHexColor { unsigned char edge; unsigned char inner; };
struct CkDebugHex {
    int artId;
    int anchorTile;
    int tile;
    DebugHexColor color;

    void setState(int newArtId, DebugHexColor newColor) {
        artId = newArtId;
        color = newColor;
    }
};

constexpr int ckdbgBLOCKER    = 996;
constexpr int ckdbgWALKABLE   = 997;
constexpr int ckdbgTRANSITION = 998;
constexpr int ckdbgSELECTED   = 999;

constexpr DebugHexColor ckdbgRED { 135, 135 };
constexpr DebugHexColor ckdbgGREEN { 215, 215 };
constexpr DebugHexColor ckdbgBLUE { 105, 105 };
constexpr DebugHexColor ckdbgYELLOW { 58, 58 };

void blit_debug_hex_colored(
		const unsigned char* src, int width, int height, int srcPitch,
		unsigned char* dest, int destX, int destY, int destPitch,
		unsigned char edgeColor, unsigned char innerColor
);

CkDebugHex* ck_debug_overlay_find_hex(int tile);

std::vector<int> ck_debug_overlay_selected_tiles();

void ck_debug_overlay_render_clear();

void ck_debug_overlay_add_hex(int fid, int anchorTile, int tile, DebugHexColor color);
void ck_debug_overlay_remove_hex(int tile);
void ck_debug_overlay_persistent_hexes(fallout::Rect* rect);

int ck_debug_overlay_build_interface_fid(int fid);

#endif
