#ifndef CK_INPUT_H
#define CK_INPUT_H

enum CkKey {
	CK_KEY_NONE = 0,
    CK_KEY_MINUS,
    CK_KEY_EQUALS,

	CK_KEY_Q,
	CK_KEY_W,
	CK_KEY_E,
	CK_KEY_X,
	CK_KEY_V,
	CK_KEY_F,
	CK_KEY_G,
	CK_KEY_H,
	CK_KEY_J,
	CK_KEY_Y,
	CK_KEY_T,
	CK_KEY_C,
	CK_KEY_U,
	CK_KEY_D,

    CK_KEY_COUNT
};

namespace ck::input {
    void update();

    bool pressed(CkKey key);
    bool just_pressed(CkKey key);

    CkKey just_pressed_key();

    void reset();

    bool shift();
    bool ctrl();
    bool alt();

    bool holding_left_mouse_button();
}

#endif
