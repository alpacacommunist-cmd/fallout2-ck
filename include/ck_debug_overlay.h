// ck_debug_overlay.h
#ifndef CK_DEBUG_OVERLAY_H
#define CK_DEBUG_OVERLAY_H

namespace fallout 
{
struct Rect;
}

void ck_debug_overlay_toggle();
bool ck_debug_overlay_enabled();

void ck_debug_overlay_render(fallout::Rect* rect);

#endif
