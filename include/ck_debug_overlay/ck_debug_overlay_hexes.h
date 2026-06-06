#ifndef CK_DEBUG_OVERLAY_HEXES_H
#define CK_DEBUG_OVERLAY_HEXES_H

#include <vector>
#include <map>
#include "ck_debug_overlay/ck_debug_overlay_render.h"

struct DebugHexColor {
    unsigned char edge;
    unsigned char inner;
};

enum class HexState {
    BLOCKER,
    WALKABLE,
    TRANSITION,
    SELECTED,
    CUSTOM
};

constexpr DebugHexColor ckdbgRED      { 135, 135 };
constexpr DebugHexColor ckdbgGREEN    { 215, 215 };
constexpr DebugHexColor ckdbgBLUE     { 105, 105 };
constexpr DebugHexColor ckdbgYELLOW   { 58, 58 };

DebugHexColor ck_debug_get_color_for_state(HexState state);

struct ckDebugHex {
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

void ck_debug_overlay_add_hex(int tile, HexState state);
void ck_debug_overlay_add_custom_hex(int tile, DebugHexColor color);
void ck_debug_overlay_remove_hex(int tile);
void ck_debug_overlay_clear_hexes();

ckDebugHex* ck_debug_overlay_find_hex(int tile);
std::vector<ckDebugHex*> ck_debug_overlay_selected_hexes();
std::vector<int> ck_debug_overlay_selected_tiles();

const std::map<int, ckDebugHex>& ck_debug_overlay_get_all_hexes();

#endif // CK_DEBUG_OVERLAY_HEXES_H
