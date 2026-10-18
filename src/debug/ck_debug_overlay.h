// ck_debug_overlay.h
#ifndef CK_DEBUG_OVERLAY_H
#define CK_DEBUG_OVERLAY_H

extern "C" int ck_map_get_id();

enum class ExportMode {
    FULL_DUMP,
	LUA_TILES,
	COUNT = 2
};

namespace fallout {
#define MOUSE_EVENT_LEFT_BUTTON_REPEAT 0x04
	struct Rect;
}

namespace ck::debug_overlay {
    bool enabled();
    void toggle();

    void render(fallout::Rect* rect);
}

#endif
