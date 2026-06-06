#include "ck_input.h"
#include "kb.h"

static int ck_to_scancode(CkKey key) {
    switch (key) {
		case CK_KEY_NONE:      return 0;
		case CK_KEY_MINUS:     return SDL_SCANCODE_MINUS;
        case CK_KEY_EQUALS:    return SDL_SCANCODE_EQUALS;

		case CK_KEY_Q:         return SDL_SCANCODE_Q;
		case CK_KEY_W:         return SDL_SCANCODE_W;
		case CK_KEY_E:         return SDL_SCANCODE_E;
		case CK_KEY_X:         return SDL_SCANCODE_X;
		case CK_KEY_V:         return SDL_SCANCODE_V;
		case CK_KEY_F:         return SDL_SCANCODE_F;
		case CK_KEY_G:         return SDL_SCANCODE_G;
		case CK_KEY_H:         return SDL_SCANCODE_H;
		case CK_KEY_J:         return SDL_SCANCODE_J;
		case CK_KEY_Y:         return SDL_SCANCODE_Y;
		case CK_KEY_T:         return SDL_SCANCODE_T;
		case CK_KEY_C:         return SDL_SCANCODE_C;

        default:                        return 0; // SDL_SCANCODE_UNKNOWN;
    }
}

static bool g_previous_keys[512] = { false };

void ck_input_update() {
    for (int i = 0; i < CK_KEY_COUNT; ++i) {
        int scancode = ck_to_scancode(static_cast<CkKey>(i));
        if (scancode > 0 && scancode < 512)
            g_previous_keys[scancode] = (fallout::gPressedPhysicalKeys[scancode] != KEY_STATE_UP);
    }
}

bool ck_input_pressed(CkKey key) {
    int scancode = ck_to_scancode(key);

    if (scancode == 0) return false;

    return fallout::gPressedPhysicalKeys[scancode] != KEY_STATE_UP;
}

bool ck_input_just_pressed(CkKey key) {
    int scancode = ck_to_scancode(key);
    if (scancode <= 0 || scancode >= 512) return false;

    bool is_currently_pressed = (fallout::gPressedPhysicalKeys[scancode] != KEY_STATE_UP);
    bool was_previously_pressed = g_previous_keys[scancode];

    return is_currently_pressed && !was_previously_pressed;
}

CkKey ck_input_get_just_pressed_key() {
    for (int i = 0; i < CK_KEY_COUNT; ++i) {
        CkKey key = static_cast<CkKey>(i);

		if (key == CK_KEY_NONE) continue;
        if (ck_input_just_pressed(key)) return key;
    }

    return CK_KEY_NONE;
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
