// ck_debug_overlay.h
#ifndef CK_DEBUG_OVERLAY_H
#define CK_DEBUG_OVERLAY_H

namespace fallout {
#define MOUSE_EVENT_LEFT_BUTTON_REPEAT 0x04
	struct Rect;

	int  mouseGetEvent();
	void mouseGetPosition(int* out_x, int* out_y);

	int  tileGetTileInDirection(int tile, int rotation, int distance);
	int  tileFromScreenXY(int x, int y, bool ignoreBounds);
	void tileWindowRefresh();

	void displayMonitorAddMessage(const char* str);
	int showDialogBox(const char* title, const char** body, int bodyLength, int x, int y, int titleColor, const char* secondaryButtonText, int bodyColor, int flags);
}

void ck_debug_overlay_toggle();
bool ck_debug_overlay_enabled();

void ck_debug_overlay_render(fallout::Rect* rect);

#endif
