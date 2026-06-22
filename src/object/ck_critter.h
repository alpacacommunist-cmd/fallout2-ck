// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"

namespace fallout {
	int textObjectAdd(Object* object, char* string, int font, int color, int outlineColor, Rect* rect);
	void tileWindowRefreshRect(Rect* rect, int elevation);

    int reg_anim_begin(int requestOptions);
    int reg_anim_end();
    int animationIsBusy(Object* obj);
    int animationRegisterMoveToTile(Object* obj, int tile, int elevation, int a4, int a5);
    int animationRegisterAnimate(Object* obj, int anim_id, int a3);


	extern int gElevation;
	extern unsigned char _colorTable[32768];
}

namespace ck {
	fallout::Object* create_critter(int pid, int tile);
	int register_critter(int pid, int tile);

	int critter_stat(fallout::Object* critter, int stat);
	int critter_pc_stat(int stat);
}

CK_API void ck_critter_float_msg(int lua_id, const char* text, int msg_type);
CK_API int ck_critter_register(int pid, int tile, const char* tag);

CK_API int ck_anim_begin(void* ptr, int weapon_ready);
CK_API int ck_anim_move_to(void* ptr, int tile, int elevation);
CK_API int ck_anim_play(void* ptr, int anim_id);
CK_API bool ck_critter_is_busy(void* ptr);
CK_API int ck_anim_end();

#endif
