#ifndef CK_DEBUG_OVERLAY_RENDER_H
#define CK_DEBUG_OVERLAY_RENDER_H

#include <vector>

namespace fallout {
    struct Rect;
}

enum class HexState {
    BLOCKER,
    WALKABLE,
    TRANSITION,
    SELECTED,
    CUSTOM
};

struct DebugHexColor {
    unsigned char edge;
    unsigned char inner;
};

constexpr DebugHexColor ckdbgRED      { 135, 135 };
constexpr DebugHexColor ckdbgGREEN    { 215, 215 };
constexpr DebugHexColor ckdbgBLUE     { 105, 105 };
constexpr DebugHexColor ckdbgYELLOW   { 58, 58 };

DebugHexColor ck_debug_get_color_for_state(HexState state);

struct CkDebugHex {
    int tile;
    HexState state;
	DebugHexColor customColor{0, 0};

    void switchTo(HexState newState) {
        state = newState;
    }

	void switchToCustom(DebugHexColor color) {
		state = HexState::CUSTOM;
		customColor = color;
	}
};

constexpr int CK_DEBUG_HEX_ART_ID = 999;

void blit_debug_hex_colored(
    const unsigned char* src, int width, int height, int srcPitch,
    unsigned char* dest, int destX, int destY, int destPitch
);

void ck_debug_overlay_render_clear();
void ck_debug_overlay_persistent_hexes(fallout::Rect* rect);
int ck_debug_overlay_build_interface_fid(int artId);

#endif // CK_DEBUG_OVERLAY_RENDER_H
