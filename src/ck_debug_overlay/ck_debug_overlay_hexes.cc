#include "ck_debug_overlay/ck_debug_overlay_hexes.h"

static std::map<int, CkDebugHex> gPersistentHexes;

DebugHexColor ck_debug_get_color_for_state(HexState state) {
    switch (state) {
        case HexState::BLOCKER:    return ckdbgRED;
        case HexState::WALKABLE:   return ckdbgBLUE;
        case HexState::TRANSITION: return ckdbgYELLOW;
        case HexState::SELECTED:   return ckdbgGREEN;
    }
    return ckdbgBLUE;
}

void ck_debug_overlay_add_hex(int tile, HexState state) {
    gPersistentHexes[tile] = { tile, state, {0,0} };
}

void ck_debug_overlay_add_custom_hex(int tile, DebugHexColor color) {
    gPersistentHexes[tile] = { tile, HexState::CUSTOM, color };
}

void ck_debug_overlay_remove_hex(int tile) {
    gPersistentHexes.erase(tile);
}

void ck_debug_overlay_clear_hexes() {
    gPersistentHexes.clear();
}

CkDebugHex* ck_debug_overlay_find_hex(int tile) {
    auto it = gPersistentHexes.find(tile);
    if (it != gPersistentHexes.end()) return &(it->second);
    return nullptr;
}

std::vector<int> ck_debug_overlay_selected_tiles() {
    std::vector<int> result;
    for (const auto& [tile, hex] : gPersistentHexes) {
        if (hex.state == HexState::SELECTED || hex.state == HexState::TRANSITION) {
            result.push_back(hex.tile);
        }
    }
    return result;
}

const std::map<int, CkDebugHex>& ck_debug_overlay_get_all_hexes() {
    return gPersistentHexes;
}
