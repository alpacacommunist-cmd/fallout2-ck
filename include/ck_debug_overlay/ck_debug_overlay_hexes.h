#ifndef CK_DEBUG_OVERLAY_HEXES_H
#define CK_DEBUG_OVERLAY_HEXES_H

#include <vector>
#include <map>
#include "ck_debug_overlay/ck_debug_overlay_render.h"

void ck_debug_overlay_add_hex(int tile, HexState state);
void ck_debug_overlay_add_custom_hex(int tile, DebugHexColor color);
void ck_debug_overlay_remove_hex(int tile);
void ck_debug_overlay_clear_hexes();

CkDebugHex* ck_debug_overlay_find_hex(int tile);
std::vector<int> ck_debug_overlay_selected_tiles();

const std::map<int, CkDebugHex>& ck_debug_overlay_get_all_hexes();

#endif // CK_DEBUG_OVERLAY_HEXES_H
