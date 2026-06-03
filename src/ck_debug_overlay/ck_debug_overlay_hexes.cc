#include "ck_debug_overlay/ck_debug_overlay_hexes.h"
#include <map>

static std::map<int, CkDebugHex> gPersistentHexes;

void ck_debug_overlay_add_hex(int artId, int tile, DebugHexColor color) {
    gPersistentHexes[tile] = { artId, tile, color };
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
        if (hex.artId == ckdbgSELECTED || hex.artId == ckdbgTRANSITION) {
            result.push_back(hex.tile);
        }
    }
    return result;
}

const std::map<int, CkDebugHex>& ck_debug_overlay_get_all_hexes() {
    return gPersistentHexes;
}
