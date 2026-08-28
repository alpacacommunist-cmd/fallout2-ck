#include "ck_ids.h"
#include "ck_proto_registry.h"
#include "ck_proto_item.h"

#include <unordered_map>
#include <cstring>

#include "ck_log.h"
static const Logger logger("CK Proto Registry");

namespace fallout {
    const size_t _proto_sizes[11] = {
        sizeof(ItemProto),    // 0x84
        sizeof(CritterProto), // 0x1A0
        sizeof(SceneryProto), // 0x38
        sizeof(WallProto),    // 0x24
        sizeof(TileProto),    // 0x1C
        sizeof(MiscProto),    // 0x1C
        0, 0, 0, 0, 0,
    };
}

namespace ck::proto {
    namespace {
        int g_next_proto_sid = ck::ids::CK_PROTO_SID_START;
        int g_next_proto_pid = ck::ids::CK_PID_START;

        std::unordered_map<int, int> g_proto_sid_to_pid; // sid -> pid for ck_script
    }

    bool is_supported_proto_type(int type) {
        auto object_type = static_cast<ck::ids::ObjectType>(type);

        return object_type == ck::ids::ObjectType::ITEM || object_type == ck::ids::ObjectType::CRITTER;
    }

    int next_proto_pid() { return g_next_proto_pid++; }
    int next_proto_sid() { return g_next_proto_sid++; }

    void link_sid_to_pid(int sid, int pid) { g_proto_sid_to_pid[sid] = pid; }
    void unlink_sid(int sid) { g_proto_sid_to_pid.erase(sid); }

    fallout::Proto* build_generic_prototype(int source_pid, int assigned_pid, int object_type) {
        fallout::Proto* src_proto = nullptr;
        if (fallout::protoGetProto(source_pid, &src_proto) != 0 || src_proto == nullptr) {
            logger.error("Failed to get source prototype for PID: {}", source_pid);
            return nullptr;
        }

        size_t proto_size = fallout::_proto_sizes[object_type];
        void* allocated_mem = std::malloc(proto_size);
        if (!allocated_mem) {
            logger.error("Out of memory when allocating custom prototype!");
            return nullptr;
        }

        std::memcpy(allocated_mem, src_proto, proto_size);
        fallout::Proto* custom_proto = static_cast<fallout::Proto*>(allocated_mem);

        custom_proto->pid = assigned_pid;
        custom_proto->messageId = assigned_pid * 100;

        return custom_proto;
    }

    void counters_reset() {
        g_next_proto_pid = ck::ids::CK_PID_START;
        g_next_proto_sid = ck::ids::CK_PROTO_SID_START;
        g_proto_sid_to_pid.clear();

        logger.debug("counters reset");
    }

    void registry_clear() {
        counters_reset();
        ck::proto::item::clear();

        logger.info("registry cleared");
    }

    int get_custom_proto(int pid, fallout::Proto** protoPtr) {
        for (const auto& node : g_custom_protos) {
            if (node.pid == pid) {
                *protoPtr = node.memory.get();
                return 0;
            }
        }

        return -1;
    }

    int register_prototype(int source_pid, int object_type, const char* lua_tag, const CustomProtoFFI& ffi_data) {
        std::string tag(lua_tag);

        if (!is_supported_proto_type(object_type)) {
            logger.error("Cannot register prototype '{}': Unsupported or invalid object type {}!", tag, object_type);
            return -1;
        }

        for (auto& proto : registry_protos) {
            if (proto.lua_tag == tag) return proto.pid;
        }

        if (g_next_proto_pid >= ck::ids::CK_PID_LIMIT) {
            logger.error("Cannot register prototype '{}': Custom item PIDs limit reached!", tag);
            return -1;
        }

        CustomProto proto;
        proto.pid         = 0;
        proto.source_pid  = source_pid;
        proto.object_type = object_type;

        proto.weight      = ffi_data.weight;
        proto.price       = ffi_data.price;

        proto.name        = std::string(ffi_data.name);
        proto.description = std::string(ffi_data.description);

        proto.inv_fid     = ffi_data.inv_fid;
        proto.ground_fid  = ffi_data.ground_fid;

        proto.usable      = ffi_data.usable;

        proto.lua_tag     = tag;
        proto.mod_id      = std::string(ck::dispatcher::current_mod_context());

        registry_protos.push_back(proto);

        if (build_custom_prototype(registry_protos.back(), g_next_proto_pid)) {
            int allocated_pid = g_next_proto_pid;
            g_next_proto_pid++;

            logger.info("Successfully registered prototype (type {}) '{}' with PID: {}", object_type, tag, allocated_pid);
            return allocated_pid;
        }

        registry_protos.pop_back();
        return -1;
    }

    int bind_prototype_script(int pid) {
        for (const auto& [sid, registered_pid] : g_proto_sid_to_pid) {
            if (registered_pid == pid) return sid;
        }

        if (g_next_proto_sid >= ck::ids::CK_PROTO_SID_LIMIT) {
            logger.error("Prototype SIDs limit reached!");
            return -1;
        }

        int assigned_sid = g_next_proto_sid++;
        int proto_sid    = ck::ids::make_proto_sid(assigned_sid);

        g_proto_sid_to_pid[assigned_sid] = pid;

        fallout::Proto* generic_proto = nullptr;
        if (ck::proto::get_custom_proto(pid, &generic_proto) == 0 && generic_proto) {
            fallout::ItemProto* item_proto = reinterpret_cast<fallout::ItemProto*>(generic_proto);
            item_proto->sid = proto_sid;
        }

        logger.debug("Bound custom prototype PID {} to SID {} ({})", pid, assigned_sid, proto_sid);
        return proto_sid;
    }

    int get_pid_by_tag(const std::string& lua_tag) {
        for (const auto& proto : registry_protos) {
            if (proto.lua_tag == lua_tag) return proto.pid;
        }
        return -1;
    }

    int get_pid_by_sid(int sid) {
        int clean_sid = ck::ids::clean_sid(sid);

        auto it = g_proto_sid_to_pid.find(clean_sid);
        if (it != g_proto_sid_to_pid.end()) return it->second;

        return -1;
    }

    int get_sid_by_pid(int pid) {
        for (const auto& [clean_sid, registered_pid] : g_proto_sid_to_pid) {
            if (registered_pid == pid) return ck::ids::make_proto_sid(clean_sid);
        }

        return -1;
    }

    bool has_pid(int pid) {
        for (const auto& proto : registry_protos) {
            if (proto.pid == pid) return true;
        }

        return false;
    }

    const std::vector<CustomProto>& get_all_protos() {
        return registry_protos;
    }
}

int ck_proto_register(int source_pid, int object_type, const char* lua_tag, const CustomProtoFFI* ffi_data) {
    if (!lua_tag || !ffi_data) return -1;
    return ck::proto::register_prototype(source_pid, object_type, lua_tag, *ffi_data);
}

int ck_proto_get_pid_by_tag(const char* lua_tag) {
    if (!lua_tag) return -1;
    return ck::proto::get_pid_by_tag(std::string(lua_tag));
}

int ck_proto_bind(int pid) {
    if (!ck::ids::is_ck_pid(pid)) return -1;
    return ck::proto::bind_prototype_script(pid);
}
