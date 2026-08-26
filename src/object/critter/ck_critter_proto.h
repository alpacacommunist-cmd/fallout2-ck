// src/object/ck_critter_proto.h
#ifndef CK_CRITTER_PROTO_H
#define CK_CRITTER_PROTO_H

#include "ck_api.h"

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
    void clear_custom_prototypes();
    bool has_custom_prototype(int pid);

    int allocate(int base_pid, const CritterLuaProtoParams* params);
    int get_gender(fallout::Object* critter);
}

CK_API int  ck_critter_allocate_prototype(int base_pid, const CritterLuaProtoParams* params);
CK_API bool ck_critter_has_custom_prototype(fallout::Object* critter);
CK_API fallout::CritterProto* ck_critter_get_proto_by_pid(int pid);

#endif
