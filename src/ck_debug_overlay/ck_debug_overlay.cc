#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_hexes.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"

#include <iostream>
#include <unordered_set>

#include "display_monitor.h"
#include "mouse.h"
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

	CkDebugHex* hex = ck_debug_overlay_find_hex(currentMouseTile);

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
    if (ck_input_ctrl() && ck_input_just_pressed(CK_KEY_MINUS)) {
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
}

static void ck_place_perimeter_blockers(const std::vector<int>& selectedTiles) {
    std::unordered_set<int> selectedSet(selectedTiles.begin(), selectedTiles.end());

    const auto& allHexes = ck_debug_overlay_get_all_hexes();

    for (const auto& [tile, hex] : allHexes) {
        if (hex.state != HexState::TRANSITION) continue;

        for (int dir = 0; dir < 6; dir++) {
            int neighbor = fallout::tileGetTileInDirection(tile, dir, 1);

            if (selectedSet.count(neighbor)) continue;  // inside selected area - skip
            if (fallout::tileIsEdge(neighbor)) continue;

            fallout::Object* existing = fallout::_obj_blocking_at(nullptr, neighbor, fallout::gElevation);
            if (existing != nullptr) continue;  // has blocker outside - skip

            fallout::Object* blocker = nullptr;
            fallout::objectCreateWithFidPid(&blocker, 0x2000015, 0x2000158);
            fallout::objectSetLocation(blocker, neighbor, fallout::gElevation, nullptr);
        }
    }
}

static void mode_main_clear_selected() {
    if (ck_input_shift() && ck_input_just_pressed(CK_KEY_Q)) {
        std::vector<int> selected = ck_debug_overlay_selected_tiles();

		ck_place_perimeter_blockers(selected);

        std::cout << "[CK DEBUG] --- Area clear started --- Count: " << selected.size() << std::endl;
        for (int tile : selected) {
			fallout::Object* blocker = fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
			bool blocking = (blocker != nullptr && (FID_TYPE(blocker->fid) != fallout::OBJ_TYPE_CRITTER));

			if (!blocking) continue;

			fallout::Rect rect;
			fallout::objectDestroy(blocker, &rect);
        }
        std::cout << "[CK DEBUG] --- Area clear complete ---" << std::endl;

		ck_place_perimeter_blockers(selected);
    }
}


static void mode_main() {
	mode_main_dude_scan();
	mode_main_paint();
	mode_main_export();

	mode_main_clear_selected();

	ck_input_update(); // key just pressed
}


void ck_debug_overlay_render(fallout::Rect* rect) {
	if (!gDebugOverlayEnabled) return;

	ck_debug_overlay_persistent_hexes(rect);

	// shift + lmb to select area
	// ctrl + lmb to clear selection
	mode_main();

	// shift + lmb to paint
	// lclick to get color
	// mode_palette();
}

