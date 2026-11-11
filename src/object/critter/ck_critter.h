// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"

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
    fallout::Object* spawn(int pid, int tile, CritterLuaSpawnParams* spawn_params);
    void assign_script(fallout::Object* critter, int script_index);
    bool kill(int lua_id);
}

CK_API fallout::Object* ck_critter_spawn(int pid, int tile, CritterLuaSpawnParams* spawn_params);
CK_API int ck_anim_begin(void* ptr, int weapon_ready);
CK_API int ck_take_out_weapon(fallout::Object* critter);
CK_API int ck_critter_anim_move_to(fallout::Object* critter, int tile, int elevation, int delay);
CK_API int ck_critter_anim_play(fallout::Object* critter, int anim_id, int delay);
CK_API int ck_critter_anim_clear(fallout::Object* critter);
CK_API int ck_anim_end();
CK_API bool ck_critter_is_busy(fallout::Object* critter);
CK_API bool ck_critter_process_turn(fallout::Object* critter, int lua_id);

CK_API bool ck_critter_bind(fallout::Object* object);
CK_API bool ck_critter_kill(fallout::Object* object);
CK_API void ck_critter_handle_dead_critter_spawn(int id, int pid, int tile);

#endif
