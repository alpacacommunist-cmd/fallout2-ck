#ifndef CK_RENDERING_H
#define CK_RENDERING_H

namespace fallout {
	struct Rect;
}

// engine entrypoint
void ck_rendering_render(fallout::Rect* rect);

// lua api
void ck_rendering_draw_scenery(int fid, int x, int y);

void ck_rendering_add_scenery(int fid, int tile, int offsetX, int offsetY);
void ck_rendering_add_tile(int fid, int tile, int offsetX, int offsetY);
void ck_rendering_clear();

#endif
