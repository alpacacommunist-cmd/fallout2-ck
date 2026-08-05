#ifndef CK_PROTO_CACHE_H
#define CK_PROTO_CACHE_H

#include "ck_api.h"

#include <string>

namespace ck::proto {
    struct CustomProtoInfo {
        int pid;
        int source_pid;
        int object_type;

        std::string lua_tag;
        std::string mod_id;
    };

    void registry_init();

    void registry_clear();

    int register_prototype(int base_pid, int object_type, const std::string& lua_tag);

    const CustomProtoInfo* find_by_pid(int pid);
}

CK_API int ck_proto_register(int base_pid, int object_type, const char* lua_tag);
CK_API int ck_proto_set_item_cost(int pid, int new_cost);
CK_API int ck_proto_set_item_weight(int pid, int new_weight);

#endif
