// src/object/ck_critter.h
#ifndef CK_CRITTER_H
#define CK_CRITTER_H

#include "ck_api.h"
#include <string>

struct CritterLua {
    int         lua_id;
    const char* mod_id;
};

struct CritterLuaProtoParams {
    const char* name;
    const char* description;
};

namespace fallout {
    struct Object;
}

namespace ck::critter {
    bool has_custom_prototype(int pid);
    void clear_custom_prototypes();
    int allocate_unique_proto(int base_pid, const std::string& lua_tag, const CritterLuaProtoParams* params);

    fallout::Object* create(int pid, int tile, int elevation);
    CritterLua spawn(int pid, int tile, int elevation, const char* tag, const CritterLuaProtoParams* params);
    bool kill(int lua_id);
}

CK_API CritterLua ck_critter_spawn(int pid, int tile, int elevation, const char* tag, const CritterLuaProtoParams* params);
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
