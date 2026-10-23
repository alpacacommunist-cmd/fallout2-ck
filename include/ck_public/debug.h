#ifndef CK_PUBLIC_DEBUG_H
#define CK_PUBLIC_DEBUG_H

namespace fallout {
	struct Rect;
}

namespace ck::debug {
    bool enabled();
    void toggle();

    void render(fallout::Rect* rect);
}

#endif
