// src/object/ck_critter_proto.h
#ifndef CK_CRITTER_PROTO_H
#define CK_CRITTER_PROTO_H

#include "ck_api.h"
#include <string>

struct CritterLuaProtoParams {
    const char* name;
    const char* description;
    const char* ai_packet;
    int team;
};

namespace fallout {
    struct Object;
    struct CritterProto;
}

namespace ck::critter::proto {
    void clear_prototypes();
    void clear_prototypes_for_mod(const std::string& mod_id);
    bool has_custom_prototype(int pid);

    int allocate(int base_pid, const CritterLuaProtoParams* params);
    int get_gender(fallout::Object* critter);
}

CK_API int  ck_critter_allocate_prototype(int base_pid, const CritterLuaProtoParams* params);
CK_API bool ck_critter_has_custom_prototype(fallout::Object* critter);
CK_API fallout::CritterProto* ck_critter_get_proto_by_pid(int pid);

CK_API int  ck_critter_proto_get_base_stat(fallout::CritterProto* proto, int stat_id);
CK_API void ck_critter_proto_set_base_stat(fallout::CritterProto* proto, int stat_id, int value);

CK_API int  ck_critter_proto_get_skill(fallout::CritterProto* proto, int skill_id);
CK_API void ck_critter_proto_set_skill(fallout::CritterProto* proto, int skill_id, int value);

#endif
