#ifndef CK_PUBLIC_RENDERING_H
#define CK_PUBLIC_RENDERING_H

namespace fallout {
	struct Rect;
}

namespace ck::rendering {
    void floor(fallout::Rect* rect);
    void roof(fallout::Rect* rect);
}

#endif
