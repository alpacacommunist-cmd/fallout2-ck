#ifndef CK_PUBLIC_DEBUG_H
#define CK_PUBLIC_DEBUG_H

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
        void process_event(void *event);
        void render(void *renderer);

        bool want_capture_mouse();
        bool want_capture_keyboard();
    }
#else
    constexpr bool tools_enabled = false;

    inline bool enabled() { return false; };
    inline void toggle() {};
    inline void render(fallout::Rect* rect) {};

    namespace ui {
        inline void process_event(void *event) {};
        inline void render(void *renderer) {};

        inline bool want_capture_mouse() { return false; };
        inline bool want_capture_keyboard() { return false; };
    }
#endif
}

#endif
