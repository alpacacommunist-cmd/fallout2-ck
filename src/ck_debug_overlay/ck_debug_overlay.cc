#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"
#include "ck_debug_overlay/ck_debug_overlay_colors.h"

#include <unordered_set>

#include "display_monitor.h"
#include "mouse.h"
#include "debug.h"
#include "tile.h"
#include "object.h"


static bool gDebugOverlayEnabled = false;

static std::unordered_set<int> gScannedTiles;

static void mode_main(int anchorTile) {
	static int sLastDudeTile = -1;

	fallout::Object* dude = fallout::gDude;

    if (dude == nullptr) return;
	if (dude->tile == sLastDudeTile) return;

	sLastDudeTile = dude->tile;

    int anchorScreenX, anchorScreenY;
    fallout::tileToScreenXY(anchorTile, &anchorScreenX, &anchorScreenY);

    const int radius = 3;

    for (int tile = dude->tile - radius;
         tile <= dude->tile + radius;
         tile++) {

        if (!fallout::hexGridTileIsValid(tile)) continue;

		// already scanned
        if (gScannedTiles.find(tile) != gScannedTiles.end()) continue;

        gScannedTiles.insert(tile);

        int screenX, screenY;
        fallout::tileToScreenXY(tile, &screenX, &screenY);

        fallout::Object* blocker = fallout::_obj_blocking_at( nullptr, tile, fallout::gElevation);

        if (blocker != nullptr && blocker->fid != dude->fid) {
            fallout::debugPrint("[CK] BLOCKER tile=%d pid=%d fid=%d flags=%08X\n",
                tile, blocker->pid, blocker->fid, blocker->flags);

            ck_debug_overlay_add_hex(996, anchorTile, screenX - anchorScreenX, screenY - anchorScreenY, RED);
        } else {
            ck_debug_overlay_add_hex(997, anchorTile, screenX - anchorScreenX, screenY - anchorScreenY, BLUE);
        }
    }
}

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
		gScannedTiles.clear();
		ck_debug_overlay_shutdown();
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

	mode_main(17290);
	if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
		// working_hover(17290);
		// palette_paint_hover(17290);
	}
}

