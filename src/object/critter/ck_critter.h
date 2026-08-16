// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"

struct CritterLua {
    int         lua_id;
    const char* mod_id;
};

struct CritterLuaProtoParams {
    const char* name;
    const char* description;
};

extern "C" const char* ck_get_current_mod_id();

namespace fallout {
    struct Object;
    struct Rect;
    struct Proto;

	extern int gElevation;

	enum CombatState : unsigned int;
    enum ObjectType : int;
    enum Gender : int;

	extern CombatState gCombatState;

	int mapGetCurrentMap();

	void _combat_delete_critter(Object* obj);
	void _combat_ai(Object* a1, Object* a2);
	bool _combatai_want_to_join(Object* a1);
    void _combat_turn_run();

	int proto_new(int* pid, fallout::ObjectType type);
	int proto_copy_proto(int srcPid, int dstPid);
    int protoGetProto(int pid, Proto** protoPtr);

	void critterKill(Object* critter, int anim, bool refreshRect);

	int tileGetTileInDirection(int tile, int rotation, int distance);

    Object* objectFindFirstAtLocation(int elevation, int tile);
    Object* objectFindNextAtLocation();
}

namespace ck {
	fallout::Object* create_critter(int pid, int tile);
	CritterLua register_critter(int pid, int tile, const char* tag, const CritterLuaProtoParams* params);

	bool critter_kill(int lua_id);
}

int ck_map_get_id();

CK_API bool ck_in_combat();
CK_API CritterLua ck_critter_register(int pid, int tile, const char* tag, const CritterLuaProtoParams* params);
CK_API bool ck_critter_in_combat();
CK_API int ck_anim_begin(void* ptr, int weapon_ready);
CK_API int ck_anim_move_to(void* ptr, int tile, int elevation);
CK_API int ck_anim_play(void* ptr, int anim_id);
CK_API int ck_anim_clear(void* ptr);
CK_API int ck_anim_end();
CK_API int  ck_critter_get_gender(void* ptr);
CK_API bool ck_critter_is_busy(void* ptr);
CK_API bool ck_critter_process_turn(void* ptr, int lua_id);
CK_API bool ck_critter_kill(int lua_id);

#endif
