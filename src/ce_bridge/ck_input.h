#ifndef CK_INPUT_H
#define CK_INPUT_H

enum CkKey {
    CK_KEY_MINUS,
    CK_KEY_EQUALS,
    CK_KEY_COMMA,
    CK_KEY_PERIOD,

    CK_KEY_COUNT
};

bool ck_input_pressed(CkKey key);

bool ck_input_shift();
bool ck_input_ctrl();
bool ck_input_alt();


#endif
