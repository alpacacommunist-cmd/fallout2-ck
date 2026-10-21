#ifndef CK_PROTO_REGISTRY_H
#define CK_PROTO_REGISTRY_H

#include "ck_api.h"
#include "utils/fallout/types/proto.h"

// passed FROM lua
struct ItemProtoFFI {
    int weight = -1;
    int price = -1;

    int inv_fid = -1;
    int ground_fid = -1;

    bool usable = false;

    const char *name;
    const char *description;
};

namespace ck::proto::registry {
    enum class SyncMode {
        Prepare,
        Restore
    };

    using ItemCallback = void(*)(const utils::fallout::types::Proto& proto, void* user_data);

    namespace item {
        // int get_proto(int pid, fallout::Proto** protoPtr);
        bool has_pid(int pid);

        int get_pid_by_tag(const std::string& lua_tag);

        int register_item_prototype(int source_pid, const char* lua_tag, const ItemProtoFFI& ffi_data);

        void sync_custom_items_on_map(SyncMode mode);

        void clear();
        void clear_for_mod(const std::string& mod_id);
    }

    int get_custom_proto(int pid, void** protoPtr);

    int build_generic_prototype(int source_pid, const char* lua_tag);
    utils::fallout::types::Proto* get(int pid);

    void for_each_item(ItemCallback callback, void* user_data);

    void counters_reset();
    void registry_clear();

    int bind_prototype_script(int pid);

    int get_pid_by_tag(const std::string& lua_tag);
    int get_pid_by_sid(int sid);
    int get_sid_by_pid(int pid);

    bool has_pid(int pid);
}

CK_API int ck_proto_get_pid_by_tag(const char* lua_tag);
CK_API int ck_proto_bind(int pid);
CK_API int ck_item_proto_register(int source_pid, const char* lua_tag, const ItemProtoFFI* ffi_data);

#endif
