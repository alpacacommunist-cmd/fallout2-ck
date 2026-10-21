#include "ck_ids.h"
#include "ck_proto_registry.h"
#include "utils/fallout/types/proto.h"

#include <unordered_map>
#include <cstring>

#include "log.h"
static constexpr logging::Context logger{"ck_proto_registry.cc"};

namespace ck::proto {
    using namespace ck::utils::fallout::types;

    namespace {
        int g_next_proto_sid = ck::ids::CK_PROTO_SID_START;
        int g_next_proto_pid = ck::ids::CK_PID_START;

        std::vector<Proto> g_registry_protos; // fallout proto ptr + meta
        std::vector<ItemProto> g_registry_item_protos; // attributes from lua

        std::unordered_map<int, int> g_proto_sid_to_pid; // sid -> pid for ck_script

        bool pid_occupied(int pid) {
            // cache locality?
            if (ck::proto::item::has_pid(pid)) return true;
            // if (ck::proto::critter::has_pid(pid)) return true;
            return false;
        }

        bool sid_occupied(int sid) {
            return g_proto_sid_to_pid.count(sid) > 0;
        }
    }

    void clear_for_mod(const std::string &mod_id) {
        for (auto &proto : g_registry_protos) {
            auto reg_it = std::ranges::find_if(
                g_registry_item_protos.begin(), g_registry_item_protos.end(),
                [&](const ItemProto &item) { return item.pid == proto.pid && item.mod_id == mod_id; }
            );

            if (reg_it != g_registry_item_protos.end()) {
                ck::proto::unlink_sid(proto.get_sid());

                ck::utils::fallout::types::proto::free(proto);
            }
        }

        // Чистим сами векторы от элементов этого мода
        std::erase_if(g_registry_protos, [](const auto &p) { return p.fallout_proto_ptr == nullptr; });
        std::erase_if(g_registry_item_protos, [&](const ItemProto &item) { return item.mod_id == mod_id; });
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

    int build_generic_prototype(int source_pid, int assigned_pid) {
        Proto ck_prototype = ck_proto::allocate(source_pid, assigned_pid);

        return ck_prototype.pid;
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
        if (protoPtr == nullptr) return -1;

        if (ck::proto::item::get_proto(pid, protoPtr) == 0) {
            return 0;
        }

        // if (ck::proto::critter::get_critter_proto(pid, protoPtr) == 0) {
        //     return 0;
        // }

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
        int pid = ck::proto::item::get_pid_by_tag(lua_tag);
        if (pid != -1) return pid;

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
        if (ck::proto::item::find_by_pid(pid)) return true;

        return false;
    }
}

int ck_proto_get_pid_by_tag(const char* lua_tag) {
    if (!lua_tag) return -1;
    return ck::proto::get_pid_by_tag(std::string(lua_tag));
}

int ck_proto_bind(int pid) {
    if (!ck::ids::is_ck_pid(pid)) return -1;
    return ck::proto::bind_prototype_script(pid);
}
