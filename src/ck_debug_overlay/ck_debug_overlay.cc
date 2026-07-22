#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_hexes.h"
#include "ck_debug_overlay/ck_debug_object_format.h"
#include "ck_input.h"

#include "geometry/geometry.h"
#include "geometry/landscape.h"
#include "object/ck_object.h"
#include "map/ck_map_camera_borders.h"

#include <unordered_set>

#include "game_sound.h"

#include "ck_log.h"
static const Logger log("CK DBG");

static bool gDebugOverlayEnabled = false;
static bool gNeedsRefresh = false;
static bool gCameraSquareDrawn = false;
static bool gAreaVisibilitySwitch = true;

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

		log.debug("tile={}, state={}, color= edge: {}, inner: {}", currentMouseTile, (int)hex->state,
				(int)color.edge, (int)color.inner);
	} else {
		if (!isShift) return;

		ck_debug_overlay_add_custom_hex(currentMouseTile, { currentColor, currentColor });
		currentColor++;
	}
}

// const char* get_scenery_frm_name(int fid) {
//     if (FID_TYPE(fid) != fallout::OBJ_TYPE_SCENERY) {
//         return "NOT_SCENERY";
//     }
//
//     int artId = fid & 0x0000FFFF;
//
//     const char* name = fallout::artGetFileName(fallout::OBJ_TYPE_SCENERY, artId);
//
//     return (name != nullptr) ? name : "UNKNOWN_FRM";
// }

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

	if (!ck_map_has_camera_borders(ck_map_get_id())) return;

	const CkCameraBorders& borders = ck_map_get_camera_borders(ck_map_get_id());

	// tileX = gridWidth - 1 - tile % gridWidth; -> tile % gridWidth = gridWidth - 1 - tileX;
	// tileY = tile / gridWidth; -> tile = tileY * gridWidth + (tile % gridWidth);
	int grid_width = fallout::tileGetHexGridWidth();
	auto to_tile = [grid_width](int x, int y) {
		return y * grid_width + (grid_width - 1 - x);
	};

    for (int x = borders.left; x <= borders.right; ++x) {
        ck_debug_overlay_add_hex(to_tile(x, borders.top), HexState::CAMERA);
        ck_debug_overlay_add_hex(to_tile(x, borders.bottom), HexState::CAMERA);
    }

    for (int y = borders.top; y <= borders.bottom; ++y) {
        ck_debug_overlay_add_hex(to_tile(borders.left, y), HexState::CAMERA);
        ck_debug_overlay_add_hex(to_tile(borders.right, y), HexState::CAMERA);
    }

	gNeedsRefresh = true;
	gCameraSquareDrawn = true;
}

static void ck_teleport_to_tile() {
	int mouseX, mouseY;
	fallout::mouseGetPosition(&mouseX, &mouseY);
	int currentMouseTile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

	objectSetLocation(fallout::gDude, currentMouseTile, fallout::gDude->elevation, nullptr);
	gNeedsRefresh = true;
}

static void mode_main_dude_scan() {
	static int sLastDudeTile = -1;

	fallout::Object* dude = fallout::gDude;
	if (dude == nullptr) return;
	if (dude->tile == sLastDudeTile) return;

	sLastDudeTile = dude->tile;

	const int radius = 3;

	for (int direction = 0; direction < 6; direction++) {
		for (int distance = 1; distance <= radius; distance++) {
			int tile = fallout::tileGetTileInDirection(dude->tile, direction, distance);

			if (!hexGridTileIsValid(tile)) continue;
			if (ck_debug_overlay_find_hex(tile) != nullptr) continue;

			ck_debug_overlay_add_hex(tile, ck_hex_state_for_tile(tile));
		}
	}
}

static void mode_main_paint() {
    const bool isSelectPressed = ck_input_shift() && ck_input_pressed(CK_KEY_W);
    const bool isClearPressed  = ck_input_shift() && ck_input_pressed(CK_KEY_X);

    if (!isSelectPressed && !isClearPressed) return;

    int mouseX, mouseY;
    fallout::mouseGetPosition(&mouseX, &mouseY);
    int currentMouseTile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

    if (currentMouseTile == -1) return;

    ckDebugHex* hex = ck_debug_overlay_find_hex(currentMouseTile);

    if (isSelectPressed) {
        if (hex == nullptr) {
            if (ck_is_tile_blocking(currentMouseTile))
                ck_debug_overlay_add_hex(currentMouseTile, HexState::TRANSITION);
            else
                ck_debug_overlay_add_hex(currentMouseTile, HexState::SELECTED);
            return;
        }

        switch (hex->state) {
            case HexState::BLOCKER:  hex->switch_to(HexState::TRANSITION); break;
            case HexState::WALKABLE: hex->switch_to(HexState::SELECTED);   break;
            default: break;
        }
    }

    else if (isClearPressed && hex != nullptr) {
        switch (hex->state) {
            case HexState::TRANSITION: hex->switch_to(HexState::BLOCKER); break;
            case HexState::SELECTED:   hex->switch_to(ck_hex_state_for_tile(hex->tile)); break;
            default: break;
        }
    }
}

static void mode_main_export() {
	std::vector<ckDebugHex*> selectedHexes = ck_debug_overlay_selected_hexes();
	int gridWidth = fallout::tileGetHexGridWidth();

	log.info("--- START DUMP --- Count: {}", selectedHexes.size());
	for (ckDebugHex* hex : selectedHexes) {
		int tile = hex->tile;
		int tileX = gridWidth - 1 - tile % gridWidth;
		int tileY = tile / gridWidth;

		log.info("SELECTED tile={} ({}, {})", tile, tileX, tileY);

		if (fallout::isExitGridAt(tile, fallout::gElevation)) {
			log.warn("EXIT GRID ON TILE");
		}

		fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);
		int objIndex = 0;

		while (obj != nullptr) {
			int objType = FID_TYPE(obj->fid);

			log.debug("[OBJ #{} Name: {} | Type: {}, PID: {}, FID: {}, SID: {}, Flags: {:#x}",
					objIndex, fallout::objectGetName(obj), objType, obj->pid, obj->fid, obj->sid,
					static_cast<unsigned int>(obj->flags));

			std::string data_debug = ck::debug::format_object_data(obj, objType);
			log.debug("{}", data_debug);

			obj = fallout::objectFindNextAtLocation();
		}
	}
	log.info("--- END DUMP --- ", selectedHexes.size());
}

static void mode_main_create_blockers() {
	std::vector<ckDebugHex*> selected_hexes = ck_debug_overlay_selected_hexes();

	log.debug(" --- Creating blockers --- Count: {}", selected_hexes.size());
	for (ckDebugHex* hex : selected_hexes) {
		ck_object_create_blocker_at(hex->tile);
		hex->switch_to(HexState::BLOCKER);
	}
	log.debug(" --- Creating blockers DONE ---");

	gNeedsRefresh = true;

	fallout::soundPlayFile("iisxxxx1");
}

static void mode_main_remove_selected() {
	std::vector<ckDebugHex*> selected_hexes = ck_debug_overlay_selected_hexes();

	log.debug(" --- Removing blockers --- Count: {}", selected_hexes.size());
	for (ckDebugHex* hex : selected_hexes) {
		ck_object_remove_at(hex->tile);
		hex->switch_to(ck_hex_state_for_tile(hex->tile));
	}
	log.debug(" --- Removing blockers DONE --- ");

	gNeedsRefresh = true;

	fallout::soundPlayFile("iisxxxx1");
}

static void mode_main_clear_all() {
	ck_debug_overlay_clear_hexes();

	gCameraSquareDrawn = false;
	gNeedsRefresh = true;
}

static void mode_main_clear_selected() {
	ck_debug_overlay_clear_hexes_by_state(HexState::SELECTED);
	ck_debug_overlay_clear_hexes_by_state(HexState::TRANSITION);
	fallout::soundPlayFile("iisxxxx1");

	gNeedsRefresh = true;
}

static void mode_main_toggle_hidden_in_rect() {
	std::vector<ckDebugHex*> selected_hexes = ck_debug_overlay_selected_hexes();

	std::vector<int> selected_tiles;
	for (auto* hex : selected_hexes) selected_tiles.push_back(hex->tile);

	HexRect rect = geometry_build_rect_from_points(selected_tiles);
	if (!rect.is_valid()) return;

	gAreaVisibilitySwitch = !gAreaVisibilitySwitch;

	ck_landscape_toggle_visibility_in_rect(rect, gAreaVisibilitySwitch);

	gNeedsRefresh = true;
}

static void mode_main() {
	mode_main_dude_scan();
	mode_main_paint(); // uses shift+w (select) shift+x(clear)

	int pressedKey = ck_input_get_just_pressed_key();
	switch (pressedKey) {
		case CK_KEY_E: 	   {
							   if (ck_input_shift()) mode_main_export();
							   break;
						   }
		case CK_KEY_U:     {
							   if (ck_input_shift()) mode_main_clear_all();
							   break;
						   }
		case CK_KEY_V:     {
							   if (ck_input_shift()) mode_main_toggle_hidden_in_rect();
							   break;
						   }

		case CK_KEY_C:     {
							   if (ck_input_alt()) ck_toggle_camera_square();
							   break;
						   }

		case CK_KEY_T:     {
							   if (ck_input_alt()) ck_teleport_to_tile();
							   break;
						   }
	}

	ck_input_update(); // key just pressed
}


void ck_debug_overlay_render(fallout::Rect* rect) {
	// if (showDialogBox(title, nullptr, 0, 169, 131, _colorTable[32328], nullptr, _colorTable[32328], DIALOG_BOX_YES_NO) == 0) {

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

