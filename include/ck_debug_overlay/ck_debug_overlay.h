// ck_debug_overlay.h
#ifndef CK_DEBUG_OVERLAY_H
#define CK_DEBUG_OVERLAY_H

namespace fallout {
#define MOUSE_EVENT_LEFT_BUTTON_REPEAT 0x04
#define HEX_GRID_WIDTH (200)
#define HEX_GRID_HEIGHT (200)
#define HEX_GRID_SIZE (HEX_GRID_WIDTH * HEX_GRID_HEIGHT)

	struct Rect;
	struct Object;

	extern Object* gDude;

	int  mouseGetEvent();
	void mouseGetPosition(int* out_x, int* out_y);

	int  tileGetTileInDirection(int tile, int rotation, int distance);
	int  tileFromScreenXY(int x, int y, bool ignoreBounds);
	void tileWindowRefresh();

	void displayMonitorAddMessage(const char* str);

	bool isExitGridAt(int tile, int elevation);
	char* objectGetName(Object* obj);
	Object* _obj_blocking_at(Object* excludeObj, int tile_num, int elev);
}

inline bool hexGridTileIsValid(int tile) {
	return tile >= 0 && tile < HEX_GRID_SIZE;
}

void ck_debug_overlay_toggle();
bool ck_debug_overlay_enabled();

void ck_debug_overlay_render(fallout::Rect* rect);

#endif
