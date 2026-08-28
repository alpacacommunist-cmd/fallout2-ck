#include "ck_ids.h"
#include "ck_utils.h"

#include "object/critter/ck_critter_proto.h"
#include "ck_messages/ck_messages.h"

#include "obj_types.h"
#include "proto_types.h"

#include <unordered_map>
#include <vector>

#include "ck_log.h"
static const Logger logger("CK Critter Proto");

namespace fallout {
    int protoGetProto(int pid, fallout::Proto** proto);
	int proto_new(int* pid, fallout::ObjectType type);
	int proto_copy_proto(int srcPid, int dstPid);

    int combat_ai_packet_num_by_name(const char* name);

    int critterGetStat(Object* critter, Stat stat);
}

namespace ck {
    namespace dispatcher {
        const char* current_mod_context();
    }
}

namespace ck::critter::proto {
    static std::unordered_map<int, fallout::CritterProto*> g_custom_prototypes;
    static std::unordered_map<std::string, std::vector<int>> g_mod_allocated_pids;

    void clear_prototypes() {
        g_custom_prototypes.clear();
    }

    void clear_prototypes_for_mod(const std::string& mod_id) {
        auto it = g_mod_allocated_pids.find(mod_id);
        if (it != g_mod_allocated_pids.end()) {
            for (int pid : it->second) {
                g_custom_prototypes.erase(pid);
            }

            g_mod_allocated_pids.erase(it);
            logger.debug("Cleared prototypes tracker for mod: {}", mod_id);
        }
    }

    bool has_custom_prototype(int pid) {
        return g_custom_prototypes.count(pid) > 0;
    }

    fallout::CritterProto* get_proto_by_pid(int pid) {
        auto it = g_custom_prototypes.find(pid);
        if (it != g_custom_prototypes.end()) return it->second;

        return nullptr;
    }

    int allocate(int base_pid, const CritterLuaProtoParams* params) {
        int pid = 0;

        if (fallout::proto_new(&pid, static_cast<fallout::ObjectType>(ck::ids::ObjectType::CRITTER)) != 0) {
            logger.error("Couldn't allocate new prototype for '{}'", base_pid);
            return -1;
        }

        if (fallout::proto_copy_proto(base_pid, pid) != 0) {
            logger.error("Couldn't copy prototype data for '{}'", base_pid);
            return -1;
        }

        fallout::Proto* generic_proto        = nullptr;
        fallout::CritterProto* critter_proto = nullptr;

        if (fallout::protoGetProto(pid, &generic_proto) == 0 && generic_proto) {
            critter_proto = reinterpret_cast<fallout::CritterProto*>(generic_proto);

            int clean_pid = ck::ids::clean_pid(pid);

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
            logger.error("Failed to get generic proto for allocated PID: {}", pid);
            return -1;
        }

        logger.info("Created unique prototype for '{}' PID: {}", base_pid, pid);
        g_custom_prototypes[pid] = critter_proto;

        std::string mod_id = dispatcher::current_mod_context();
        g_mod_allocated_pids[mod_id].push_back(pid);

        return pid;
    }
}

int ck_critter_allocate_prototype(int base_pid, const CritterLuaProtoParams* params) {
    return ck::critter::proto::allocate(base_pid, params);
}

bool ck_critter_has_custom_prototype(fallout::Object* critter) {
    CK_ENSURE_VALID_OBJECT(critter);

    return ck::critter::proto::has_custom_prototype(critter->pid);
}

fallout::CritterProto* ck_critter_get_proto_by_pid(int pid) {
    return ck::critter::proto::get_proto_by_pid(pid);
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
    if (proto) {
        proto->data.baseStats[stat_id] = value;
        logger.debug("Proto PID {} stat {} changed to {}", proto->pid, stat_id, value);
    }
}

void ck_critter_proto_set_skill(fallout::CritterProto* proto, int skill_id, int value) {
    if (proto) {
        proto->data.skills[skill_id] = value;
        logger.debug("Proto PID {} skill {} changed to {}", proto->pid, skill_id, value);
    }
}
