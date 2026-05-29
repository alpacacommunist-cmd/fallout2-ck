// ck_debug_rendering.cc
#include <iostream>

#include "ck_debug_rendering.h"

#include "debug.h"
#include "mouse.h"
#include "tile.h"

void ck_debug_rendering_capture_offsets(int anchorTile) {
    int anchorScreenX;
    int anchorScreenY;

    fallout::tileToScreenXY(anchorTile, &anchorScreenX, &anchorScreenY);

    static int lastMouseX = -9999;
    static int lastMouseY = -9999;

    int mouseX;
    int mouseY;
    fallout::mouseGetPosition(&mouseX, &mouseY);

	if (mouseX == lastMouseX && mouseY == lastMouseY) {
		return;
	}

    lastMouseX = mouseX;
    lastMouseY = mouseY;

    if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) == 0) {
        return;
    }

    int relativeX = mouseX - anchorScreenX;
    int relativeY = mouseY - anchorScreenY;

	std::cout << "{fid = 192, x = " << relativeX << ", y = " << relativeY << "}, " << std::endl;
}
