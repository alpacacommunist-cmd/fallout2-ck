// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"
#include "ck_critter_proto.h"

namespace fallout {
    struct Object;
}

// passed from lua
struct CritterLuaSpawnParams {
    const char* tag;
    int elevation;
    int script_index;
    int team;
};

namespace ck::critter {
    bool has_custom_prototype(int pid);
    void clear_spawn_queues();

	int spawn(int pid, int tile, CritterLuaSpawnParams* spawn_params, const CritterLuaProtoParams* params);
    bool kill(int lua_id);
}

CK_API int ck_critter_spawn(int pid, int tile, CritterLuaSpawnParams* spawn_params, const CritterLuaProtoParams* params);
CK_API int ck_anim_begin(void* ptr, int weapon_ready);
CK_API int ck_anim_take_out_weapon(fallout::Object* critter, int delay);
CK_API int ck_anim_move_to(void* ptr, int tile, int elevation);
CK_API int ck_anim_play(void* ptr, int anim_id);
CK_API int ck_anim_clear(void* ptr);
CK_API int ck_anim_end();
CK_API bool ck_critter_is_busy(fallout::Object* critter);
CK_API bool ck_critter_process_turn(fallout::Object* critter, int lua_id);
CK_API bool ck_critter_kill(int lua_id);
CK_API void ck_critter_reset_spawn_counters_for_mod(const char* mod_id);

#endif
