#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"

#include <unordered_set>
#include <iostream>

#include "display_monitor.h"
#include "mouse.h"
#include "debug.h"
#include "tile.h"
#include "ck_input.h"

static bool gDebugOverlayEnabled = false;

static std::unordered_set<int> gScannedTiles;

static void mode_palette(int anchorTile) {
    if (!ck_debug_overlay_enabled()) return;

    static int lastTile = -1;
    static unsigned char currentColor = 0;

    int mouseX, mouseY;
    fallout::mouseGetPosition(&mouseX, &mouseY);

    int tile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

    if (tile == lastTile) return;

    lastTile = tile;

    int anchorScreenX, anchorScreenY;
    fallout::tileToScreenXY(anchorTile, &anchorScreenX, &anchorScreenY);

    fallout::debugPrint("[CK] color=%d tile=%d\n", currentColor, tile);

    ck_debug_overlay_add_hex(999, anchorTile, tile, { currentColor, currentColor });

    currentColor++;
}

static void mode_select(int anchorTile) {
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

	if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
		ck_debug_overlay_add_hex(999, anchorTile, tile, ckdbgGREEN);

		if (fallout::ck_input_shift()) ck_debug_overlay_remove_hex(tile);
	}
}

static void mode_main_dude_scan(int anchorTile) {
	static int sLastDudeTile = -1;

	fallout::Object* dude = fallout::gDude;

    if (dude == nullptr) return;
	if (dude->tile == sLastDudeTile) return;

	sLastDudeTile = dude->tile;

    int anchorScreenX, anchorScreenY;
    fallout::tileToScreenXY(anchorTile, &anchorScreenX, &anchorScreenY);

    const int radius = 3;

    for (int tile = dude->tile - radius; tile <= dude->tile + radius; tile++) {
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

            ck_debug_overlay_add_hex(996, anchorTile, tile, ckdbgRED);
        } else {
            ck_debug_overlay_add_hex(997, anchorTile, tile, ckdbgBLUE);
        }
    }
}

static void mode_main_paint(int anchorTile) {
	if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
		int mouseX, mouseY;
		fallout::mouseGetPosition(&mouseX, &mouseY);
		int currentMouseTile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

		CkDebugHex* hex = ck_debug_overlay_find_hex(currentMouseTile);
		// SHIFT + LMB -> select
		if (fallout::ck_input_shift()) {
			if (hex == nullptr) {
				ck_debug_overlay_add_hex(ckdbgSELECTED, anchorTile, currentMouseTile, ckdbgGREEN);
			} else if (hex->artId == ckdbgBLOCKER) {
				hex->artId = ckdbgTRANSITION;
				hex->color = ckdbgYELLOW;
			} else if (hex->artId == ckdbgWALKABLE) {
				hex->artId = ckdbgSELECTED;
				hex->color = ckdbgGREEN;
			}
		}

		// CTRL + LMB -> clear selection
		if (fallout::ck_input_ctrl()) {
			if (hex == nullptr) return;

			if (hex->artId == ckdbgTRANSITION) {
				hex->artId = ckdbgBLOCKER;
				hex->color = ckdbgRED;
			} else if (hex->artId == ckdbgSELECTED) {
				// scanned and selected. Make it just scanned
				if (gScannedTiles.find(hex->tile) != gScannedTiles.end()) {
					hex->artId = ckdbgWALKABLE;
					hex->color = ckdbgBLUE;
				// selected but not scanned, just remove
				} else {
					ck_debug_overlay_remove_hex(hex->tile);
				}
			}
		}
	}
}

static void mode_main_export(int anchorTile) {
    static bool minusKeyWasPressed = false; // previous frame
    // ctrl + -
    bool isCtrl = fallout::ck_input_ctrl();
    bool isMinusPressed = fallout::ck_input_pressed(fallout::CK_KEY_MINUS);

    if (isCtrl && isMinusPressed) {
        if (!minusKeyWasPressed) {
            minusKeyWasPressed = true;

			std::vector<int> selected = ck_debug_overlay_selected_tiles();

			std::cout << "[CK DEBUG] --- START DUMP --- Count: " << selected.size() << std::endl;
			for (int tile : selected) {
				std::cout << "[CK DEBUG] SELECTED tile=" << tile << std::endl;
			}
			std::cout << "[CK DEBUG] --- END DUMP ---" << std::endl;
        }
    } else {
        minusKeyWasPressed = false;
    }
}

static void mode_main(int anchorTile) {
	mode_main_dude_scan(anchorTile);
	mode_main_paint(anchorTile);
	mode_main_export(anchorTile);
}

bool ck_debug_overlay_enabled() { return gDebugOverlayEnabled; }

void ck_debug_overlay_toggle() {
    gDebugOverlayEnabled = !gDebugOverlayEnabled;

	if (gDebugOverlayEnabled) {
		fallout::displayMonitorAddMessage("[CK] Debug Overlay: ON");
	} else {
		fallout::displayMonitorAddMessage("[CK] Debug Overlay: OFF");

		ck_debug_overlay_clear();
		gScannedTiles.clear();
	}
}

void ck_debug_overlay_render(fallout::Rect* rect) {
    if (!gDebugOverlayEnabled) return;

	ck_debug_overlay_persistent_hexes(rect);

	mode_main(17290);

	// mode_select(17290);
	// mode_palette(17290);
}

