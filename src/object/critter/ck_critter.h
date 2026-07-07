// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"

struct CritterLua {
    int         lua_id;
    const char* mod_id;
};

extern "C" const char* ck_get_current_mod_id();
int ck_dispatcher_get_state_tile(int map_id, const char* lua_tag);

namespace fallout {
	int mapGetCurrentMap();

	extern unsigned int gCombatState;
	void _combat_delete_critter(Object* obj);

	int proto_new(int* pid, int type);
	int proto_copy_proto(int srcPid, int dstPid);

	void critterKill(Object* critter, int anim, bool refreshRect);

	int  textObjectAdd(Object* object, char* string, int font, int color, int outlineColor, Rect* rect);
	void tileWindowRefreshRect(Rect* rect, int elevation);

    int reg_anim_begin(int requestOptions);
    int reg_anim_end();
	int reg_anim_clear(Object* a1);
    int animationIsBusy(Object* obj);
    int animationRegisterMoveToTile(Object* obj, int tile, int elevation, int a4, int a5);
    int animationRegisterAnimate(Object* obj, int anim_id, int a3);

	extern int gElevation;
	extern unsigned char _colorTable[32768];
}

namespace ck {
	fallout::Object* create_critter(int pid, int tile);
	CritterLua register_critter(int pid, int tile, const char* tag);

	bool critter_kill(int lua_id);
}

int ck_map_get_id();

CK_API bool ck_in_combat();
CK_API void ck_critter_float_msg(int lua_id, const char* text, int msg_type);
CK_API CritterLua ck_critter_register(int pid, int tile, const char* tag);
CK_API bool ck_critter_in_combat();
CK_API int ck_anim_begin(void* ptr, int weapon_ready);
CK_API int ck_anim_move_to(void* ptr, int tile, int elevation);
CK_API int ck_anim_play(void* ptr, int anim_id);
CK_API int ck_anim_clear(void* ptr);
CK_API int ck_anim_end();
CK_API bool ck_critter_is_busy(void* ptr);
CK_API bool ck_critter_kill(int lua_id);

#endif
