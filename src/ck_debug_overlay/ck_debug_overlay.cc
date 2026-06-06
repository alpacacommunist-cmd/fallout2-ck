#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_hexes.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"
#include "ck_input.h"

#include "object/ck_object.h"
#include "map/ck_map.h"

#include <iostream>
#include <unordered_set>

#include "display_monitor.h"
#include "game_sound.h"
#include "mouse.h"
#include "tile.h"

static bool gDebugOverlayEnabled = false;
static bool gNeedsRefresh = false;
static bool gCameraSquareDrawn = false;

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

	ckDebugHex* hex = ck_debug_overlay_find_hex(currentMouseTile);
	const bool isShift = ck_input_shift();

	if (hex != nullptr) {
		DebugHexColor color = (hex->state == HexState::CUSTOM)
			? hex->customColor
			: ck_debug_get_color_for_state(hex->state);

		std::cout << "[CK PALETTE] tile=" << currentMouseTile
			<< ", state=" << (int)hex->state
			<< ", color= edge: " << (int)color.edge << ", inner: " << (int)color.inner
			<< std::endl;
	} else {
		if (!isShift) return;

		ck_debug_overlay_add_custom_hex(currentMouseTile, { currentColor, currentColor });
		currentColor++;
	}
}

static bool ck_is_tile_blocking(int tile) {
    fallout::Object* blocker = fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
    return blocker != nullptr && FID_TYPE(blocker->fid) != fallout::OBJ_TYPE_CRITTER;
}

static HexState ck_hex_state_for_tile(int tile) {
    return ck_is_tile_blocking(tile) ? HexState::BLOCKER : HexState::WALKABLE;
}

static void ck_toggle_camera_square() {
	if (gCameraSquareDrawn) {
		ck_debug_overlay_clear_hexes_by_state(HexState::CAMERA);

		gCameraSquareDrawn = false;
		gNeedsRefresh = true;

		return;
	}

	const CkCameraBorders& borders = ck_map_get_camera_borders();

	if (!borders.enabled) { return; }

	// tileX = gridWidth - 1 - tile % gridWidth; -> tile % gridWidth = gridWidth - 1 - tileX;
	// tileY = tile / gridWidth; -> tile = tileY * gridWidth + (tile % gridWidth);
	int gridWidth = fallout::tileGetHexGridWidth();
	    auto toTile = [gridWidth](int x, int y) {
        return y * gridWidth + (gridWidth - 1 - x);
    };

    for (int x = borders.left; x <= borders.right; ++x) {
        ck_debug_overlay_add_hex(toTile(x, borders.top), HexState::CAMERA);
        ck_debug_overlay_add_hex(toTile(x, borders.bottom), HexState::CAMERA);
    }

    for (int y = borders.top; y <= borders.bottom; ++y) {
        ck_debug_overlay_add_hex(toTile(borders.left, y), HexState::CAMERA);
        ck_debug_overlay_add_hex(toTile(borders.right, y), HexState::CAMERA);
    }

	gNeedsRefresh = true;
	gCameraSquareDrawn = true;
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

		ck_debug_overlay_add_hex(tile, ck_hex_state_for_tile(tile));
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

	ckDebugHex* hex = ck_debug_overlay_find_hex(currentMouseTile);

	// SHIFT + LMB: select area
	if (isShift) {
		if (hex == nullptr) {
			if (ck_is_tile_blocking(currentMouseTile))
				ck_debug_overlay_add_hex(currentMouseTile, HexState::TRANSITION);
			else
				ck_debug_overlay_add_hex(currentMouseTile, HexState::SELECTED);
			return;
		}

		switch (hex->state) {
			case HexState::BLOCKER:  hex->switchTo(HexState::TRANSITION); break;
			case HexState::WALKABLE: hex->switchTo(HexState::SELECTED);   break;
			default: break;
		}
	}
	// CTRL + LMB: clear selection
	else if (isCtrl && hex != nullptr) {
		switch (hex->state) {
			case HexState::TRANSITION: hex->switchTo(HexState::BLOCKER); break;
			case HexState::SELECTED: hex->switchTo(ck_hex_state_for_tile(hex->tile)); break;

			default: break;
		}
	}
}

static void mode_main_export() {
	std::vector<ckDebugHex*> selectedHexes = ck_debug_overlay_selected_hexes();
	int gridWidth = fallout::tileGetHexGridWidth();

	std::cout << "[CK DEBUG] --- START DUMP --- Count: " << selectedHexes.size() << std::endl;
	for (ckDebugHex* hex : selectedHexes) {
		int tile = hex->tile;
		int tileX = gridWidth - 1 - tile % gridWidth, tileY = tile / gridWidth;

		std::cout << "[CK DEBUG] SELECTED tile=" << tile << ", hex(x=" << tileX << ", y=" << tileY << ")"
			<< std::endl;
	}
	std::cout << "[CK DEBUG] --- END DUMP ---" << std::endl;
}

static void mode_main_create_blockers() {
	std::vector<ckDebugHex*> selectedHexes = ck_debug_overlay_selected_hexes();

	std::cout << "[CK DEBUG] --- Creating blockers --- Count: " << selectedHexes.size() << std::endl;
	for (ckDebugHex* hex : selectedHexes) {
		ck_object_create_blocker_at(hex->tile);
		hex->switchTo(HexState::BLOCKER);
	}
	std::cout << "[CK DEBUG] --- Creating blockers COMPLETE ---" << std::endl;

	gNeedsRefresh = true;

	fallout::soundPlayFile("iisxxxx1");
}

static void mode_main_remove_selected_blockers() {
	std::vector<ckDebugHex*> selectedHexes = ck_debug_overlay_selected_hexes();

	std::cout << "[CK DEBUG] --- Removing blockers started --- Count: " << selectedHexes.size() << std::endl;
	for (ckDebugHex* hex : selectedHexes) {
		ck_object_remove_blocker_at(hex->tile);
		hex->switchTo(ck_hex_state_for_tile(hex->tile));
	}
	std::cout << "[CK DEBUG] --- Removing blockers complete ---" << std::endl;

	gNeedsRefresh = true;

	fallout::soundPlayFile("iisxxxx1");
}

static void mode_main_clear_all() {
	ck_debug_overlay_clear_hexes();

	gCameraSquareDrawn = false;
	gNeedsRefresh = true;
}


static void mode_main() {
	mode_main_dude_scan();
	mode_main_paint();

	int pressedKey = ck_input_get_just_pressed_key();
	switch (pressedKey) {
		case CK_KEY_E: 	   {
							   if (ck_input_shift()) mode_main_export();
							   break;
						   }
		case CK_KEY_X:     {
							   if (ck_input_shift()) mode_main_clear_all();
							   break;
						   }
		case CK_KEY_C:     {
							   if (ck_input_alt()) ck_toggle_camera_square();
							   break;
						   }
		case CK_KEY_MINUS: {
							   if (ck_input_ctrl()) mode_main_remove_selected_blockers();
							   break;
					       }
		case CK_KEY_EQUALS: {
								if (ck_input_ctrl()) mode_main_create_blockers();
								break;
							}

	}

	ck_input_update(); // key just pressed
}


void ck_debug_overlay_render(fallout::Rect* rect) {
	if (!gDebugOverlayEnabled) return;

	ck_debug_overlay_persistent_hexes(rect);

	// shift + lmb to select area
	// ctrl + lmb to clear selection
	mode_main();

	if (gNeedsRefresh) {
		gNeedsRefresh = false;
		fallout::tileWindowRefresh();
	}

	// shift + lmb to paint
	// lclick to get color
	// mode_palette();
}

