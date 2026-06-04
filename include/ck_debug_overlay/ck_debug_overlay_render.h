#ifndef CK_DEBUG_OVERLAY_RENDER_H
#define CK_DEBUG_OVERLAY_RENDER_H

#include <vector>

namespace fallout {
    struct Rect;
}

void blit_debug_hex_colored(
    const unsigned char* src, int width, int height, int srcPitch,
    unsigned char* dest, int destX, int destY, int destPitch
);

void ck_debug_overlay_render_clear();
void ck_debug_overlay_persistent_hexes(fallout::Rect* rect);
int ck_debug_overlay_build_interface_fid(int artId);

#endif // CK_DEBUG_OVERLAY_RENDER_H
