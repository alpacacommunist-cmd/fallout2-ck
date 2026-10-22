#ifndef CK_PROTO_REGISTRY_H
#define CK_PROTO_REGISTRY_H

#include "ck_api.h"
#include "utils/fallout/types/proto.h"

namespace fallout {
    struct Object;
}

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

// passed FROM lua
struct CritterProtoFFI {
    const char* name;
    const char* description;
    const char* ai_packet;

    int team;
};

namespace ck::proto::registry {
    enum class SyncMode {
        Prepare,
        Restore
    };

    using TypesCallback = void(*)(const utils::fallout::types::Proto& proto, void* user_data);
    void for_each_item(TypesCallback callback, void* user_data);
    void for_each_critter(TypesCallback callback, void* user_data);

    namespace item {
        int allocate(int source_pid, const char* lua_tag, const ItemProtoFFI& ffi_data);
        void sync_custom_items_on_map(SyncMode mode);
    }

    namespace critter {
        int allocate(int source_pid, const CritterProtoFFI* ffi_data);
    }

    // Engine hook
    int get_custom_proto(int pid, void** protoPtr);

    int build_generic_prototype(int source_pid, const char* lua_tag);
    utils::fallout::types::Proto* get(int pid);

    void counters_reset();
    void clear();
    void clear_for_mod(const char* mod_id);

    int bind_prototype_script(int pid);

    int get_pid_by_tag(const std::string& lua_tag);
    int get_pid_by_sid(int sid);
    int get_sid_by_pid(int pid);

    bool has_pid(int pid);
    int source_pid(int pid);
}

CK_API int ck_proto_get_pid_by_tag(const char* lua_tag);
CK_API int ck_proto_bind(int pid);
CK_API int ck_item_proto_register(int source_pid, const char* lua_tag, const ItemProtoFFI* ffi_data);
CK_API int ck_critter_allocate_prototype(int source_pid, const CritterProtoFFI* params);
CK_API bool ck_critter_has_custom_prototype(fallout::Object* critter);

using Proto = ck::utils::fallout::types::Proto;
CK_API Proto* ck_critter_get_proto_by_pid(int pid);
CK_API int ck_critter_proto_get_base_stat(Proto* proto, int stat_id);
CK_API int ck_critter_proto_get_skill(Proto* proto, int skill_id);
CK_API void ck_critter_proto_set_base_stat(Proto* proto, int stat_id, int value);
CK_API void ck_critter_proto_set_skill(Proto* proto, int skill_id, int value);

#endif
