// ck_debug_overlay.h
#ifndef CK_DEBUG_OVERLAY_H
#define CK_DEBUG_OVERLAY_H

namespace fallout {
	struct Rect;
}

namespace ck::debug {
    bool enabled();
    void toggle();

    void render(fallout::Rect* rect);
}

#endif
