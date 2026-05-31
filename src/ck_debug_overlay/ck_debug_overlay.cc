#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"
#include "ck_debug_overlay/ck_debug_overlay_colors.h"

#include <cstdlib>

#include "mouse.h"
#include "display_monitor.h"
#include "debug.h"
#include "tile.h"


static bool gDebugOverlayEnabled = false;

bool ck_debug_overlay_enabled() {
    return gDebugOverlayEnabled;
}

void ck_debug_overlay_toggle() {
    gDebugOverlayEnabled = !gDebugOverlayEnabled;

	if (gDebugOverlayEnabled) {
		fallout::displayMonitorAddMessage("[CK] Debug Overlay: ON");
	} else {
		fallout::displayMonitorAddMessage("[CK] Debug Overlay: OFF");
		ck_debug_overlay_clear();

	}
}

void working_hover(int anchorTile) {
    if (!gDebugOverlayEnabled) return; 

    static int lastTile = -1;

    int anchorScreenX, anchorScreenY;
    fallout::tileToScreenXY(anchorTile, &anchorScreenX, &anchorScreenY);

    int mouseX, mouseY;
    fallout::mouseGetPosition(&mouseX, &mouseY);

	int screenX, screenY;
    int tile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

    if (tile == lastTile) return;

    lastTile = tile;

    int gridWidth = fallout::tileGetHexGridWidth();
    int tileX = gridWidth - 1 - tile % gridWidth,
		tileY = tile / gridWidth;

	fallout::Object* blocker = fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
	bool blocked = blocker != nullptr;

	fallout::tileToScreenXY(tile, &screenX, &screenY);

	fallout::debugPrint(
			"[CK] Hover tile=%d hex=(%d,%d) screen=(%d, %d) blocked=%s\n", tile, tileX, tileY,
			screenX - anchorScreenX, screenY - anchorScreenY,
			blocked ? "true" : "false");

	ck_debug_overlay_add_hex(999, anchorTile, screenX - anchorScreenX, screenY - anchorScreenY, YELLOW);
}

void ck_debug_overlay_render(fallout::Rect* rect) {
    if (!gDebugOverlayEnabled) return;

	ck_debug_overlay_persistent_hexes(rect);
	if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
		// working_hover(17290);
		palette_paint_hover(17290);
	}
}

