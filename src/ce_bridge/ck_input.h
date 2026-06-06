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

    CK_KEY_COUNT
};

void ck_input_update();

bool ck_input_pressed(CkKey key);
bool ck_input_just_pressed(CkKey key);

CkKey ck_input_get_just_pressed_key();

bool ck_input_shift();
bool ck_input_ctrl();
bool ck_input_alt();


#endif
