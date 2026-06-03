#include "ck_input.h"
#include "kb.h"

static int ck_to_scancode(CkKey key) {
    switch (key) {
		case CK_KEY_MINUS:     return SDL_SCANCODE_MINUS;
        case CK_KEY_EQUALS:    return SDL_SCANCODE_EQUALS;

        default:                        return 0; // SDL_SCANCODE_UNKNOWN;
    }
}

bool ck_input_pressed(CkKey key) {
    int scancode = ck_to_scancode(key);

    if (scancode == 0) return false;

    return fallout::gPressedPhysicalKeys[scancode] != KEY_STATE_UP;
}

bool ck_input_shift() {
    return fallout::gPressedPhysicalKeys[SDL_SCANCODE_LSHIFT] || fallout::gPressedPhysicalKeys[SDL_SCANCODE_RSHIFT];
}

bool ck_input_ctrl() {
    return fallout::gPressedPhysicalKeys[SDL_SCANCODE_LCTRL] || fallout::gPressedPhysicalKeys[SDL_SCANCODE_RCTRL];
}

bool ck_input_alt() {
    return fallout::gPressedPhysicalKeys[SDL_SCANCODE_LALT] || fallout::gPressedPhysicalKeys[SDL_SCANCODE_RALT];
}
