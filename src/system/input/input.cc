#include "system/input/input.h"
#include "fallout2-ce/kb.h"

namespace fallout {
	int  mouseGetEvent();
#define MOUSE_EVENT_LEFT_BUTTON_REPEAT 0x04
}

static int ck_to_scancode(CkKey key) {
    switch (key) {
    case CK_KEY_NONE:   return 0;
    case CK_KEY_MINUS:  return SDL_SCANCODE_MINUS;
    case CK_KEY_EQUALS: return SDL_SCANCODE_EQUALS;

    case CK_KEY_Q:      return SDL_SCANCODE_Q;
    case CK_KEY_W:      return SDL_SCANCODE_W;
    case CK_KEY_E:      return SDL_SCANCODE_E;
    case CK_KEY_X:      return SDL_SCANCODE_X;
    case CK_KEY_V:      return SDL_SCANCODE_V;
    case CK_KEY_F:      return SDL_SCANCODE_F;
    case CK_KEY_G:      return SDL_SCANCODE_G;
    case CK_KEY_H:      return SDL_SCANCODE_H;
    case CK_KEY_J:      return SDL_SCANCODE_J;
    case CK_KEY_Y:      return SDL_SCANCODE_Y;
    case CK_KEY_T:      return SDL_SCANCODE_T;
    case CK_KEY_C:      return SDL_SCANCODE_C;
    case CK_KEY_U:      return SDL_SCANCODE_U;
    case CK_KEY_D:      return SDL_SCANCODE_D;

    default:            return 0; // SDL_SCANCODE_UNKNOWN;
    }
}

namespace ck::input {
static bool g_previous_keys[512] = {false};

void update() {
    for (int index = 0; index < CK_KEY_COUNT; ++index) {
        int scancode = ck_to_scancode(static_cast<CkKey>(index));
        if (scancode > 0 && scancode < 512)
            g_previous_keys[scancode] = (fallout::gPressedPhysicalKeys[scancode] != KEY_STATE_UP);
    }
}

bool pressed(CkKey key) {
    int scancode = ck_to_scancode(key);

    if (scancode == 0) return false;

    return fallout::gPressedPhysicalKeys[scancode] != KEY_STATE_UP;
}

bool just_pressed(CkKey key) {
    int scancode = ck_to_scancode(key);
    if (scancode <= 0 || scancode >= 512) return false;

    bool is_currently_pressed = (fallout::gPressedPhysicalKeys[scancode] != KEY_STATE_UP);
    bool was_previously_pressed = g_previous_keys[scancode];

    return is_currently_pressed && !was_previously_pressed;
}

CkKey just_pressed_key() {
    for (int index = 0; index < CK_KEY_COUNT; ++index) {
        CkKey key = static_cast<CkKey>(index);

        if (key == CK_KEY_NONE) continue;
        if (just_pressed(key)) return key;
    }

    return CK_KEY_NONE;
}

void reset() {
    for (int index = 0; index < CK_KEY_COUNT; ++index) {
        int scancode = ck_to_scancode(static_cast<CkKey>(index));
        if (scancode > 0 && scancode < 512) {
            g_previous_keys[scancode] = (fallout::gPressedPhysicalKeys[scancode] != KEY_STATE_UP);
        }
    }
}

bool shift() {
    return fallout::gPressedPhysicalKeys[SDL_SCANCODE_LSHIFT] || fallout::gPressedPhysicalKeys[SDL_SCANCODE_RSHIFT];
}

bool ctrl() {
    return fallout::gPressedPhysicalKeys[SDL_SCANCODE_LCTRL] || fallout::gPressedPhysicalKeys[SDL_SCANCODE_RCTRL];
}

bool alt() {
    return fallout::gPressedPhysicalKeys[SDL_SCANCODE_LALT] || fallout::gPressedPhysicalKeys[SDL_SCANCODE_RALT];
}

bool holding_left_mouse_button() {
    return ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) == 0);
}
} // namespace ck::input
