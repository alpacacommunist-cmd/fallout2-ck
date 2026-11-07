// ck_debug_overlay.h
#ifndef CK_DEBUG_OVERLAY_H
#define CK_DEBUG_OVERLAY_H

namespace fallout {
	struct Rect;
}

namespace ck::debug {
#ifdef CK_EDITOR_BUILD
    constexpr bool tools_enabled = true;

    bool enabled();
    void toggle();

    void render(fallout::Rect* rect);

    namespace ui {
        void initialize();
        void shutdown();
    }
#else
    constexpr bool tools_enabled = false;

    inline bool enabled() { return false; };
    inline void toggle() {};
    inline void render(fallout::Rect* rect) {};

    namespace ui {
        inline void initialize() {};
        inline void shutdown();
    }
#endif
}

#endif
