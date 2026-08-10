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
#define HEX_GRID_WIDTH (200)
#define HEX_GRID_HEIGHT (200)
#define HEX_GRID_SIZE (HEX_GRID_WIDTH * HEX_GRID_HEIGHT)
    enum Rotation : int;

	struct Rect;
	struct Object;

	extern Object* gDude;

	int  mouseGetEvent();
	void mouseGetPosition(int* out_x, int* out_y);

    int tileGetTileInDirection(int tile, Rotation rotation, int distance);

	int  tileFromScreenXY(int x, int y, bool ignoreBounds);
	void tileWindowRefresh();

	void displayMonitorAddMessage(const char* str);

	bool isExitGridAt(int tile, int elevation);
	char* objectGetName(Object* obj);
	Object* _obj_blocking_at(Object* excludeObj, int tile_num, int elev);
	int objectSetLocation(Object* obj, int tile, int elevation, Rect* rect);
}

inline bool hexGridTileIsValid(int tile) {
	return tile >= 0 && tile < HEX_GRID_SIZE;
}

bool ck_debug_overlay_enabled();
void ck_debug_overlay_toggle();

void ck_debug_overlay_render(fallout::Rect* rect);

#endif
