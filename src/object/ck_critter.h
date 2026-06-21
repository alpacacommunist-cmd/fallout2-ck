// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"

namespace fallout {
	int textObjectAdd(Object* object, char* string, int font, int color, int outlineColor, Rect* rect);
	void tileWindowRefreshRect(Rect* rect, int elevation);

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
CK_API int ck_critter_register(int pid, int tile);
CK_API int player_stat(int stat);
CK_API int player_pc_stat(int stat);

#endif
