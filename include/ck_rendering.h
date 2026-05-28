#ifndef CK_RENDERING_H
#define CK_RENDERING_H

namespace fallout {
	struct Rect;
}

void ck_rendering_draw(fallout::Rect* rect);

void ck_rendering_draw_scenery(int fid, int x, int y);

#endif
