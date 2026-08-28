#ifndef CK_PROTO_REGISTRY_H
#define CK_PROTO_REGISTRY_H

#include "ck_api.h"

#include <string>
#include <memory>

#include "proto_types.h"

struct MallocDeleter { void operator()(void* ptr) const { std::free(ptr); } };
using  UniqueProtoPtr = std::unique_ptr<fallout::Proto, MallocDeleter>;
struct ProtoNode { int pid; UniqueProtoPtr memory; };

// stored internally
struct ItemProto {
    int pid;
    int source_pid;

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

// passed FROM lua
struct ItemProtoFFI {
    int weight;
    int price;

    int inv_fid;
    int ground_fid;

    bool usable;

    const char *name;
    const char *description;
};

// passed TO lua
struct ItemProtoLuaView { int pid; const char* lua_tag; };

namespace fallout {
    int protoGetProto(int pid, fallout::Proto** proto);
}

namespace ck::common {
    const char* system_mod_id();
    const char* current_mod_id();
}

namespace ck::proto {
    enum class SyncMode {
        Prepare,
        Restore
    };

    namespace item {
        int get_proto(int pid, fallout::Proto** protoPtr);

        const ItemProto* find_by_pid(int pid);
        int find_pid_by_tag(const std::string& lua_tag);

        int register_item_prototype(int source_pid, const char* lua_tag, const ItemProtoFFI& ffi_data);

        void sync_custom_items_on_map(SyncMode mode);

        void clear();
        void clear_for_mod(const std::string& mod_id);
    }

    bool is_supported_proto_type(int type);

    int next_proto_pid();
    int next_proto_sid();

    void link_sid_to_pid(int sid, int pid);
    void unlink_sid(int sid);

    fallout::Proto* build_generic_prototype(int source_pid, int assigned_pid, int object_type);

    void counters_reset();
    void registry_clear();

    int  bind_prototype_script(int pid);

    int get_pid_by_tag(const std::string& lua_tag);
    int get_pid_by_sid(int sid);
    int get_sid_by_pid(int pid);

    bool has_pid(int pid);
}

CK_API int ck_proto_get_pid_by_tag(const char* lua_tag);
CK_API int ck_proto_bind(int pid);
CK_API int ck_item_proto_register(int source_pid, const char* lua_tag, const ItemProtoFFI* ffi_data);

#endif
