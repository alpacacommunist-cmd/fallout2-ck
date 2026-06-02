#ifndef CK_RENDERING_H
#define CK_RENDERING_H

namespace fallout {
	struct Rect;
}

// engine entrypoint
void ck_rendering_render(fallout::Rect* rect);

// lua api
void ck_rendering_draw_scenery(int fid, int x, int y);


int build_scenery_fid(int fid);
int build_tile_fid(int fid);
int build_interface_fid(int fid);

void ck_rendering_add_scenery(int fid, int tile);
void ck_rendering_add_tile(int fid, int tile);
void ck_rendering_clear();

struct CkCameraBorders {
    bool enabled = false;

    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
};

void ck_rendering_set_camera_borders(int left, int right, int top, int bottom);
void ck_rendering_clear_camera_borders();
bool ck_rendering_is_camera_position_allowed(int tile);
bool ck_rendering_has_camera_borders();

int ck_rendering_build_interface_fid(int fid);
int ck_rendering_build_tile_fid(int fid);
int ck_rendering_build_scenery_fid(int fid);

#endif
