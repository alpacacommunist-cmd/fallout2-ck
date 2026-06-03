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

bool ck_debug_overlay_enabled() { return gDebugOverlayEnabled; }

void ck_debug_overlay_toggle() {
    gDebugOverlayEnabled = !gDebugOverlayEnabled;

	if (gDebugOverlayEnabled) {
		fallout::displayMonitorAddMessage("[CK] Debug Overlay: ON");

		fallout::tileWindowRefresh();
	} else {
		fallout::displayMonitorAddMessage("[CK] Debug Overlay: OFF");

		ck_debug_overlay_render_clear();
	}
}

// different modes, work in progress

static void mode_palette() {
	if (!ck_debug_overlay_enabled()) return;
    if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) == 0) return;

    static int lastTile = -1;
    static unsigned char currentColor = 0;

    int mouseX, mouseY;
    fallout::mouseGetPosition(&mouseX, &mouseY);
    int currentMouseTile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

    if (currentMouseTile == lastTile) return;
    lastTile = currentMouseTile;
	CkDebugHex* hex = ck_debug_overlay_find_hex(currentMouseTile);

    const bool isShift = ck_input_shift();

	// clicked on existing hex
    if (hex != nullptr) {
		std::cout << "[CK PALETTE] tile=" << currentMouseTile
			<< ", color= edge: " << (int)hex->color.edge << ", inner: " << (int)hex->color.inner
			<< std::endl;
	} else {
		if (!isShift) return;

		ck_debug_overlay_add_hex(999, currentMouseTile, { currentColor, currentColor });
		currentColor++;
	}
}

static void mode_select() {
    if (!gDebugOverlayEnabled) return;

    static int lastTile = -1;

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
			screenX, screenY, blocked ? "true" : "false");

	if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
		ck_debug_overlay_add_hex(999, tile, ckdbgGREEN);

		if (ck_input_shift()) ck_debug_overlay_remove_hex(tile);
	}
}

static void mode_main_dude_scan() {
    static int sLastDudeTile = -1;

    fallout::Object* dude = fallout::gDude;
    if (dude == nullptr) return;
    if (dude->tile == sLastDudeTile) return;

    sLastDudeTile = dude->tile;

    const int radius = 3;

    for (int tile = dude->tile - radius; tile <= dude->tile + radius; tile++) {
        if (!fallout::hexGridTileIsValid(tile)) continue;

        if (ck_debug_overlay_find_hex(tile) != nullptr) continue;

        fallout::Object* blocker = fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
		bool blocking = (blocker != nullptr && (FID_TYPE(blocker->fid) != fallout::OBJ_TYPE_CRITTER));

        if (blocking) {
            ck_debug_overlay_add_hex(ckdbgBLOCKER, tile, ckdbgRED);
        } else {
            ck_debug_overlay_add_hex(ckdbgWALKABLE, tile, ckdbgBLUE);
        }
    }
}

static void mode_main_paint() {
    if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) == 0) return;

    int mouseX, mouseY;
    fallout::mouseGetPosition(&mouseX, &mouseY);
    int currentMouseTile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

    const bool isShift = ck_input_shift();
    const bool isCtrl = ck_input_ctrl();
    if (!isShift && !isCtrl) return;

    fallout::Object* blocker = fallout::_obj_blocking_at(nullptr, currentMouseTile, fallout::gElevation);
	bool blocking = (blocker != nullptr && (FID_TYPE(blocker->fid) != fallout::OBJ_TYPE_CRITTER));

    CkDebugHex* hex = ck_debug_overlay_find_hex(currentMouseTile);
    // SHIFT + LMB select area
    if (isShift) {
        if (hex == nullptr) {
			if (blocking) ck_debug_overlay_add_hex(ckdbgTRANSITION, currentMouseTile, ckdbgYELLOW);
			else ck_debug_overlay_add_hex(ckdbgSELECTED, currentMouseTile, ckdbgGREEN);
            return;
        }

        switch (hex->artId) {
            case ckdbgBLOCKER:  hex->setState(ckdbgTRANSITION, ckdbgYELLOW); break;
            case ckdbgWALKABLE: hex->setState(ckdbgSELECTED,   ckdbgGREEN);  break;
        }
    }
    // CTRL + LMB clear selection
    else if (isCtrl && hex != nullptr) {
        switch (hex->artId) {
            case ckdbgTRANSITION:
                hex->setState(ckdbgBLOCKER, ckdbgRED);
                break;

            case ckdbgSELECTED:
				if (blocking) hex->setState(ckdbgBLOCKER, ckdbgRED);
				else hex->setState(ckdbgWALKABLE, ckdbgBLUE);

                break;
        }
    }
}

static void mode_main_export() {
    static bool minusKeyWasPressed = false;

    bool isCtrl = ck_input_ctrl();
    bool isMinusPressed = ck_input_pressed(CK_KEY_MINUS);

    if (isCtrl && isMinusPressed) {
        if (!minusKeyWasPressed) {
            minusKeyWasPressed = true;

            std::vector<int> selected = ck_debug_overlay_selected_tiles();

			int gridWidth = fallout::tileGetHexGridWidth();

            std::cout << "[CK DEBUG] --- START DUMP --- Count: " << selected.size() << std::endl;
            for (int tile : selected) {
				int tileX = gridWidth - 1 - tile % gridWidth,
					tileY = tile / gridWidth;

                std::cout << "[CK DEBUG] SELECTED tile=" << tile << ", hex(x=" << tileX << ", y=" << tileY << ")"
					<< std::endl;
            }
            std::cout << "[CK DEBUG] --- END DUMP ---" << std::endl;
        }
    } else {
        minusKeyWasPressed = false;
    }
}

static void mode_main() {
	mode_main_dude_scan();
	mode_main_paint();
	mode_main_export();
}


void ck_debug_overlay_render(fallout::Rect* rect) {
    if (!gDebugOverlayEnabled) return;

	ck_debug_overlay_persistent_hexes(rect);

	// shift + lmb to select area
	// ctrl + lmb to clear selection
	mode_main();

	// mode_select(17290);

	// shift + lmb to paint
	// lclick to get color
	// mode_palette();
}

