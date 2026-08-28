#include "ck_ids.h"
#include "ck_proto_registry.h"

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

        bool pid_occupied(int pid) {
            // cache locality?
            if (ck::proto::item::has_pid(pid)) return true;
            return false;
        }

        bool sid_occupied(int sid) {
            return g_proto_sid_to_pid.count(sid) > 0;
        }
    }

    bool is_supported_proto_type(int type) {
        auto object_type = static_cast<ck::ids::ObjectType>(type);

        return object_type == ck::ids::ObjectType::ITEM || object_type == ck::ids::ObjectType::CRITTER;
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
