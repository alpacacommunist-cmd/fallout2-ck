#ifndef CK_PROTO_REGISTRY_H
#define CK_PROTO_REGISTRY_H

#include "ck_api.h"

#include <string>
#include <vector>

#include "proto_types.h"

struct CustomProtoLuaView { int pid; const char* lua_tag; };

struct CustomProto {
    int pid;
    int source_pid;
    int object_type;

    int weight;
    int price;

    std::string name;
    std::string description;

    int inv_fid;
    int ground_fid;

    bool usable;

    std::string lua_tag;
    std::string mod_id;
};

struct CustomProtoFFI {
    int weight;
    int price;

    int inv_fid;
    int ground_fid;

    bool usable;

    const char *name;
    const char *description;
};

namespace ck::proto {
    enum class SyncMode {
        Prepare,
        Restore
    };

    void sync_custom_items_on_map(SyncMode mode);

    void memory_clear();
    void registry_clear();
    void rebuild_custom_prototypes();
    int  get_custom_proto(int pid, fallout::Proto** protoPtr);

    void rebuild_tracked_items();

    int  register_prototype(int source_pid, int object_type, const char* lua_tag, const CustomProtoFFI& ffi_data);
    int  bind_prototype_script(int pid);

    int get_pid_by_tag(const std::string& lua_tag);
    int get_pid_by_sid(int sid);
    int get_sid_by_pid(int pid);

    bool has_pid(int pid);

    const CustomProto* find_by_pid(int runtime_pid);
    const std::vector<CustomProto>& get_all_protos();
}

CK_API int ck_proto_register(int source_pid, int object_type, const char* lua_tag, const CustomProtoFFI* ffi_data);
CK_API int ck_proto_get_pid_by_tag(const char* lua_tag);
CK_API int ck_proto_bind(int pid);

#endif
