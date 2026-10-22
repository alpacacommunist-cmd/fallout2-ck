#include "ck_ids.h"
#include "ck_proto_registry.h"
#include "ck_mods/ck_mods.h"

#include <algorithm>
#include <memory>
#include <cstring>
#include <unordered_map>
#include <cstring>

#include "log.h"
static constexpr logging::Context logger{"ck_proto_registry.cc"};

namespace ck::proto::registry {
    using namespace ck::utils::fallout;

    namespace {
        int g_next_proto_sid = ck::ids::CK_PROTO_SID_START;

        std::vector<std::unique_ptr<types::Proto>> g_registry_protos; // fallout proto ptr + meta
        std::unordered_map<int, int> g_proto_sid_to_pid; // sid -> pid for ck_script

        bool pid_occupied(int pid) {
            // cache locality?
            return std::ranges::any_of(g_registry_protos, [pid](const auto &proto) { return proto->pid == pid; });
        }

        bool sid_occupied(int sid) {
            return g_proto_sid_to_pid.contains(sid);
        }

        int next_proto_pid() {
            for (int pid = ck::ids::CK_PID_START; pid < ck::ids::CK_PID_LIMIT; ++pid) {
                if (!pid_occupied(pid)) return pid;
            }

            logger.error("CRITICAL: Out of custom PIDs! All slots are occupied.");
            return -1;
        }

        int next_proto_sid() {
            for (int sid = ck::ids::CK_PROTO_SID_START; sid < ck::ids::CK_PROTO_SID_LIMIT; ++sid) {
                if (!sid_occupied(sid)) return sid;
            }

            logger.error("CRITICAL: Out of custom SIDs! All slots are occupied.");
            return -1;
        }


        void link_sid_to_pid(int sid, int pid) { g_proto_sid_to_pid[sid] = pid; }
        void unlink_sid(int sid) { g_proto_sid_to_pid.erase(sid); }
    }

    types::Proto* get(int pid) {
        auto it = std::ranges::find_if(g_registry_protos, [pid](const auto &proto) { return proto->pid == pid; });

        return it != g_registry_protos.end() ? it->get() : nullptr;
    }

    void for_each_item(TypesCallback callback, void* user_data) {
        for (const auto &proto : g_registry_protos) {
            if (proto->object_type == static_cast<int>(ck::ids::ObjectType::ITEM)) {
                callback(*proto, user_data);
            }
        }
    }

    void for_each_critter(TypesCallback callback, void* user_data) {
        for (const auto &proto : g_registry_protos) {
            if (proto->object_type == static_cast<int>(ck::ids::ObjectType::CRITTER)) {
                callback(*proto, user_data);
            }
        }
    }

    void clear_for_mod(const char *mod_id) {
        for (auto &proto : g_registry_protos) {
            if (proto && proto->mod_id == mod_id) {
                unlink_sid(proto->get_sid());
            }
        }

        std::erase_if(g_registry_protos, [mod_id](const auto &p) { return p->mod_id == mod_id; });
        logger.info("Cleared registry for mod: {}", mod_id);
    }

    int build_generic_prototype(int source_pid, const char* lua_tag) {
        int assigned_pid = next_proto_pid();
        if (assigned_pid == -1) return -1;

        auto proto = std::make_unique<types::Proto>();
        if (lua_tag) proto->lua_tag = std::string(lua_tag);

        int status = types::proto::allocate(source_pid, assigned_pid, ck::mods::current_mod_id(), *proto);
        if (status == -1) return -1;

        g_registry_protos.push_back(std::move(proto));

        return assigned_pid;
    }

    void counters_reset() {
        g_next_proto_sid = ck::ids::CK_PROTO_SID_START;
        g_proto_sid_to_pid.clear();

        logger.debug("counters reset");
    }

    void clear() {
        g_registry_protos.clear(); 
        counters_reset();

        logger.info("registry cleared");
    }

    // ENGINE HOOK
    // int protoGetProto(const ProtoId& protoId, Proto** protoPtr)
    int get_custom_proto(int pid, void** protoPtr) {
        if (protoPtr == nullptr) return -1;

        types::Proto* proto = get(ck::ids::clean_pid(pid));
        if (proto) {
            *protoPtr = proto->fallout_proto_ptr;
            return 0;
        }

        logger.error("Proto not found: {}", pid);
        *protoPtr = nullptr;
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
        int proto_sid = ck::ids::make_proto_sid(assigned_sid);

        g_proto_sid_to_pid[assigned_sid] = pid;

        auto it = std::ranges::find_if(g_registry_protos, [pid](const auto &proto) { return proto->pid == pid; });
        if (it != g_registry_protos.end()) {
            it->get()->set_sid(proto_sid);
        }

        logger.debug("Bound custom prototype PID {} to SID {} ({})", pid, assigned_sid, proto_sid);
        return proto_sid;
    }

    int get_pid_by_tag(const std::string& lua_tag) {
        auto it = std::ranges::find_if(g_registry_protos, [lua_tag](const auto &proto) { return proto->lua_tag == lua_tag; });

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
        types::Proto* proto = get(ck::ids::clean_pid(pid));
        return proto != nullptr;
    }

    int source_pid(int pid) {
        types::Proto* proto = get(ck::ids::clean_pid(pid));
        return proto ? proto->source_pid : -1;
    }
}

// FFI
int ck_proto_get_pid_by_tag(const char* lua_tag) {
    if (!lua_tag) return -1;
    return ck::proto::registry::get_pid_by_tag(std::string(lua_tag));
}

int ck_proto_bind(int pid) {
    if (!ck::ids::is_ck_pid(pid)) return -1;
    return ck::proto::registry::bind_prototype_script(pid);
}
