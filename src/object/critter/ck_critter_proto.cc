#include "ck_ids.h"
#include "ck_utils.h"

#include "object/critter/ck_critter_proto.h"
#include "ck_messages/ck_messages.h"

#include "obj_types.h"
#include "proto_types.h"

#include <unordered_set>

#include "ck_log.h"
static const Logger logger("CK Critter Proto");

namespace fallout {
    int protoGetProto(int pid, fallout::Proto** proto);
	int proto_new(int* pid, fallout::ObjectType type);
	int proto_copy_proto(int srcPid, int dstPid);

    int combat_ai_packet_num_by_name(const char* name);

    int critterGetStat(Object* critter, Stat stat);
}

namespace ck::critter::proto {
    static std::unordered_set<int> g_custom_prototypes;

    void clear_custom_prototypes() {
        g_custom_prototypes.clear();
    }

    bool has_custom_prototype(int pid) {
        return g_custom_prototypes.count(pid) > 0;
    }

    int allocate(int base_pid, const CritterLuaProtoParams* params) {
        int unique_pid = 0;

        if (fallout::proto_new(&unique_pid, static_cast<fallout::ObjectType>(ck::ids::object_types::CRITTER)) != 0) {
            logger.error("Couldn't allocate new prototype for '{}'", base_pid);
            return -1;
        }

        if (fallout::proto_copy_proto(base_pid, unique_pid) != 0) {
            logger.error("Couldn't copy prototype data for '{}'", base_pid);
            return -1;
        }

        fallout::Proto* generic_proto = nullptr;
        if (fallout::protoGetProto(unique_pid, &generic_proto) == 0 && generic_proto) {
            fallout::CritterProto* critter_proto = reinterpret_cast<fallout::CritterProto*>(generic_proto);

            int clean_pid = ck::ids::clean_pid(unique_pid);

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
            logger.error("Failed to get generic proto for allocated PID: {}", unique_pid);
            return -1;
        }

        logger.info("Created unique prototype for '{}' PID: {}", base_pid, unique_pid);
        g_custom_prototypes.insert(unique_pid);

        return unique_pid;
    }

    int get_gender(fallout::Object* critter) {
        fallout::Gender gender = static_cast<fallout::Gender>(fallout::critterGetStat(critter, fallout::STAT_GENDER));
        return static_cast<int>(gender);
    }
}

int ck_critter_allocate_prototype(int base_pid, const CritterLuaProtoParams* params) {
    return ck::critter::proto::allocate(base_pid, params);
}

bool ck_critter_has_custom_prototype(fallout::Object* critter) {
    CK_ENSURE_VALID_OBJECT(critter);

    return ck::critter::proto::has_custom_prototype(critter->pid);
}

