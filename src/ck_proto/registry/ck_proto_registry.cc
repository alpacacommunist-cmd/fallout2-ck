#include "ck_ids.h"
#include "ck_proto_registry.h"
#include "ck_mods/ck_mods.h"

#include <algorithm>
#include <memory>
#include <unordered_map>

#include "log.h"
static constexpr logging::Context logger{"ck_proto_registry.cc"};

namespace ck::proto::registry {
    using namespace ck::utils::fallout;

    namespace {
        std::vector<std::unique_ptr<Proto>> protos; // fallout proto ptr + meta
        std::unordered_map<int, int> g_proto_sid_to_pid; // sid -> pid for ck_script

        void link_sid_to_pid(int sid, int pid) { g_proto_sid_to_pid[sid] = pid; }
        void unlink_sid(int sid) { g_proto_sid_to_pid.erase(sid); }
    }

    Proto* get(int pid) {
        auto it = std::ranges::find_if(protos, [pid](const auto &proto) { return proto->pid == pid; });
        return it != protos.end() ? it->get() : nullptr;
    }

    void for_each_item(TypesCallback callback, void* user_data) {
        for (const auto &proto : protos) {
            if (proto->object_type == static_cast<int>(ck::ids::ObjectType::ITEM)) {
                callback(*proto, user_data);
            }
        }
    }

    void for_each_critter(TypesCallback callback, void* user_data) {
        for (const auto &proto : protos) {
            if (proto->object_type == static_cast<int>(ck::ids::ObjectType::CRITTER)) {
                callback(*proto, user_data);
            }
        }
    }

    int bind_prototype_script(Proto* proto) {
        if (!proto) {
            logger.error("Couldn't bind, pid not found: {}", proto->pid);
        }

        for (const auto& [sid, registered_pid] : g_proto_sid_to_pid) {
            if (registered_pid == proto->pid) return sid;
        }

        int assigned_sid = ck::ids::next_script_index(ck::mods::current_mod_id(), proto->object_type);

        g_proto_sid_to_pid[assigned_sid] = proto->pid;
        proto->set_sid(assigned_sid);

        logger.debug("Bound custom prototype PID {} to script_index {}", proto->pid, assigned_sid);
        return assigned_sid;
    }

    Proto* build_generic_prototype(int source_pid, const char* lua_tag) {
        const char* mod_id = ck::mods::current_mod_id();
        int object_type = types::proto::type(source_pid);

        int assigned_pid = ck::ids::next_proto_id(mod_id, object_type);
        if (assigned_pid == -1) return nullptr;

        auto proto = std::make_unique<Proto>();
        if (types::proto::allocate(source_pid, assigned_pid, *proto) != 0) return nullptr;

        if (lua_tag) proto->lua_tag = std::string(lua_tag);
        proto->mod_id = mod_id;

        Proto* proto_ptr = proto.get();
        protos.push_back(std::move(proto));

        bind_prototype_script(proto_ptr);

        return proto_ptr;
    }

    void clear() {
        protos.clear();

        logger.info("registry cleared");
    }

    void clear_for_object_type(int object_type) {
        const char* type_name = ck::ids::object_type_to_string(object_type);
        logger.info("Clearing prototypes for object_type: {}", type_name);

        for (auto &proto : protos) {
            if (proto && proto->object_type == object_type) {
                ck::ids::free_script_index(proto->get_sid());
                ck::ids::free_proto_id(proto->pid);

                unlink_sid(proto->get_sid());
            }
        }

        size_t count = std::erase_if(protos, [object_type](const auto &proto) { return proto->object_type == object_type; });
        logger.debug("Successfully removed {} prototypes of type '{}' from registry", count, type_name);
    }

    void clear_for_mod(const char *mod_id) {
        logger.info("Clearing prototypes for mod: {}", mod_id);

        for (auto &proto : protos) {
            if (proto && proto->mod_id == mod_id) {
                ck::ids::free_script_index(proto->get_sid());
                ck::ids::free_proto_id(proto->pid);

                unlink_sid(proto->get_sid());
            }
        }

        size_t count = std::erase_if(protos, [mod_id](const auto &proto) { return proto->mod_id == mod_id; });
        logger.debug("Hot Reload: Cleared {} prototypes for mod '{}'", count, mod_id);
    }

    // ENGINE HOOK
    // int protoGetProto(const ProtoId& protoId, Proto** protoPtr)
    int get_custom_proto(int pid, void** protoPtr) {
        if (protoPtr == nullptr) return -1;

        Proto* proto = get(ck::ids::clean_pid(pid));
        if (proto) {
            *protoPtr = proto->fallout_proto_ptr;
            return 0;
        }

        logger.error("Proto not found: {}", pid);
        *protoPtr = nullptr;
        return -1;
    }

    int get_pid_by_tag(const std::string& lua_tag) {
        auto it = std::ranges::find_if(protos, [lua_tag](const auto &proto) { return proto->lua_tag == lua_tag; });

        return -1;
    }

    int get_pid_by_sid(int sid) {
        int clean_sid = ck::ids::clean_sid(sid);

        auto it = g_proto_sid_to_pid.find(clean_sid);
        if (it != g_proto_sid_to_pid.end()) return it->second;

        return -1;
    }

    // int get_sid_by_pid(int pid) {
    //     for (const auto& [clean_sid, registered_pid] : g_proto_sid_to_pid) {
    //         if (registered_pid == pid) {
    //             Proto* proto = get(pid);
    //             return proto ? ck::ids::make_proto_sid(clean_sid, proto->object_type) : -1;
    //         }
    //     }
    //
    //     return -1;
    // }

    bool has_pid(int pid) {
        Proto* proto = get(ck::ids::clean_pid(pid));
        return proto != nullptr;
    }

    int source_pid(int pid) {
        Proto* proto = get(ck::ids::clean_pid(pid));
        return proto ? proto->source_pid : -1;
    }
}

// FFI
int ck_proto_get_pid_by_tag(const char* lua_tag) {
    if (!lua_tag) return -1;
    return ck::proto::registry::get_pid_by_tag(std::string(lua_tag));
}

