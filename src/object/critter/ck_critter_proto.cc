#include "ck_ids.h"
#include "utils.h"

#include "object/critter/ck_critter_proto.h"
#include "ck_messages/ck_messages.h"

#include "obj_types.h"
#include "proto.h"

#include <unordered_map>
#include <vector>

#include "log.h"
static const Logger logger("CK Critter Proto");

namespace fallout {
    int combat_ai_packet_num_by_name(const char* name);

    int critterGetStat(Object* critter, Stat stat);
}

namespace ck {
    namespace common {
        const char* current_mod_id();
    }
}

namespace ck::critter::proto {
    // pid -> proto pointers
    static std::unordered_map<int, fallout::CritterProto*> g_custom_prototypes;
    // pid -> source_pid map
    static std::unordered_map<int, int> g_source_pid_mappings;
    // mod_id -> pids (vector)
    static std::unordered_map<const char*, std::vector<int>> g_mod_allocated_pids;

    void clear_prototypes() {
        g_custom_prototypes.clear();
    }

    void clear_prototypes_for_mod(const char* mod_id) {
        auto it = g_mod_allocated_pids.find(mod_id);
        if (it == g_mod_allocated_pids.end()) return;

        for (int pid : it->second) {
            g_custom_prototypes.erase(pid);
            g_source_pid_mappings.erase(pid);
        }

        g_mod_allocated_pids.erase(it);
        logger.debug("Cleared prototypes tracker for mod: {}", mod_id);
    }

    bool is_custom(int pid) {
        return g_custom_prototypes.count(pid) > 0;
    }

    int proto_sid(int pid) {
        fallout::Proto* generic_proto = nullptr;

        if (fallout::protoGetProto(fallout::ProtoId{pid}, &generic_proto) == 0 && generic_proto) {
            if (ck::ids::clean_sid(generic_proto->critter.sid) != -1) {
                return generic_proto->critter.sid;
            }
        }

        return -1;
    }

    fallout::CritterProto* get(int pid) {
        auto it = g_custom_prototypes.find(pid);
        if (it != g_custom_prototypes.end()) return it->second;

        return nullptr;
    }

    int source_pid(int pid) {
        auto it = g_source_pid_mappings.find(pid);
        if (it != g_source_pid_mappings.end()) return it->second;

        return -1;
    }

    int allocate(int source_pid, const CritterLuaProtoParams* params) {
        fallout::ProtoId source {source_pid};
        fallout::ProtoId ck_proto;

        if (fallout::proto_new(ck_proto, static_cast<fallout::ObjectType>(ck::ids::ObjectType::CRITTER)) != 0) {
            logger.error("Couldn't allocate new prototype (base PID: {})", source_pid);
            return -1;
        }

        if (fallout::proto_copy_proto(source, ck_proto) != 0) {
            logger.error("Couldn't copy prototype data for '{}'", source_pid);
            return -1;
        }

        fallout::Proto* generic_proto        = nullptr;
        fallout::CritterProto* critter_proto = nullptr;

        if (fallout::protoGetProto(ck_proto, &generic_proto) == 0 && generic_proto) {
            critter_proto = reinterpret_cast<fallout::CritterProto*>(generic_proto);

            int clean_pid = ck::ids::clean_pid(ck_proto.pid());

            int msg_name_id = clean_pid * 100;
            int msg_desc_id = msg_name_id + 1;

            critter_proto->messageId = msg_name_id;

            if (!utils::is_blank(params->name)) {
                ck::messages_add_string("pro_crit.msg", msg_name_id, params->name);
            }

            if (!utils::is_blank(params->description)) {
                ck::messages_add_string("pro_crit.msg", msg_desc_id, params->description);
            }

            if (params && !ck::utils::is_blank(params->ai_packet)) {
                int ai_id = fallout::combat_ai_packet_num_by_name(params->ai_packet);

                if (ai_id != -1) {
                    critter_proto->aiPacket = ai_id;
                    logger.debug("Assigned AI packet '{}' (ID: {}) to unique proto", params->ai_packet, ai_id);
                } else {
                    logger.warn("AI packet '{}' not found in game data! Using base proto AI.", params->ai_packet);
                }
            }
        } else {
            logger.error("Failed to get generic proto for allocated PID: {}", ck_proto.pid());
            return -1;
        }

        g_custom_prototypes[ck_proto.pid()] = critter_proto;
        g_source_pid_mappings[ck_proto.pid()] = source_pid;

        const char* mod_id = common::current_mod_id();
        g_mod_allocated_pids[mod_id].push_back(ck_proto.pid());

        logger.info("Created unique prototype for '{}' PID: {}", source_pid, ck_proto.pid());

        return ck_proto.pid();
    }
}

int ck_critter_allocate_prototype(int base_pid, const CritterLuaProtoParams* params) {
    return ck::critter::proto::allocate(base_pid, params);
}

bool ck_critter_has_custom_prototype(fallout::Object* critter) {
    CK_ENSURE_VALID_OBJECT(critter);

    return ck::critter::proto::is_custom(critter->pid);
}

fallout::CritterProto* ck_critter_get_proto_by_pid(int pid) {
    return ck::critter::proto::get(pid);
}

int ck_critter_proto_get_base_stat(fallout::CritterProto* proto, int stat_id) {
    if (proto) return proto->data.baseStats[stat_id];
    return -1;
}

int ck_critter_proto_get_skill(fallout::CritterProto* proto, int skill_id) {
    if (proto) return proto->data.skills[skill_id];
    return -1;
}

void ck_critter_proto_set_base_stat(fallout::CritterProto* proto, int stat_id, int value) {
    if (proto == nullptr) return;

    proto->data.baseStats[stat_id] = value;
    logger.debug("Proto PID {} stat {} changed to {}", proto->pid, stat_id, value);
}

void ck_critter_proto_set_skill(fallout::CritterProto* proto, int skill_id, int value) {
    if (proto == nullptr) return;

    proto->data.skills[skill_id] = value;
    logger.debug("Proto PID {} skill {} changed to {}", proto->pid, skill_id, value);
}
