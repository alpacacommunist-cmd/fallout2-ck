#include "utils.h"
#include "ck_ids.h"

#include "engine/messages.h"
#include "ck_proto_registry.h"

// TODO: utils/types wrapper
#include "fallout2-ce/obj_types.h"

#include "log.h"
static constexpr logging::Context logger{"ck_item_proto.cc"};

namespace fallout {
    int combat_ai_packet_num_by_name(const char* name);
}

namespace ck::proto::registry::critter {
    using namespace ck::utils::fallout;

    int allocate(int source_pid, const CritterProtoFFI* ffi_data) {
        int assigned_pid = registry::build_generic_prototype(source_pid, nullptr);
        if (assigned_pid == -1) return -1;

        types::Proto* proto = ck::proto::registry::get(assigned_pid);

        if (ffi_data->ai_packet) {
            int ai_id = fallout::combat_ai_packet_num_by_name(ffi_data->ai_packet);

            if (ai_id != -1) {
                proto->set_critter_ai(ai_id);
                logger.debug("Assigned AI packet '{}' (ID: {}) to unique proto", ffi_data->ai_packet, ai_id);
            } else {
                logger.warning("AI packet '{}' not found in game data! Using base proto AI.", ffi_data->ai_packet);
            }
        }

        int msg_name_id = proto->get_msg_id();
        if (!utils::is_blank(ffi_data->name)) {
            ck::messages_add_string("pro_crit.msg", msg_name_id, ffi_data->name);
        }

        if (!utils::is_blank(ffi_data->description)) {
            ck::messages_add_string("pro_crit.msg", msg_name_id + 1, ffi_data->description);
        }

        return proto->packed_pid();
    }
}

// FFI
using Proto = ck::utils::fallout::types::Proto;

int ck_critter_allocate_prototype(int source_pid, const CritterProtoFFI* params) {
    return ck::proto::registry::critter::allocate(source_pid, params);
}

bool ck_critter_has_custom_prototype(fallout::Object* critter) {
    CK_ENSURE_VALID_OBJECT(critter);

    return ck::proto::registry::get(ck::ids::clean_pid(critter->pid)) != nullptr;
}

Proto* ck_critter_get_proto_by_pid(int pid) {
    return ck::proto::registry::get(ck::ids::clean_pid(pid));
}

int ck_critter_proto_get_base_stat(Proto* proto, int stat_id) {
    if (proto) return proto->get_critter_base_stat(stat_id);
    return -1;
}

int ck_critter_proto_get_skill(Proto* proto, int skill_id) {
    if (proto) return proto->get_critter_skill(skill_id);
    return -1;
}

void ck_critter_proto_set_base_stat(Proto* proto, int stat_id, int value) {
    if (proto == nullptr) return;

    proto->set_critter_base_stat(stat_id, value);
    logger.debug("Proto PID {} stat {} changed to {}", proto->pid, stat_id, value);
}

void ck_critter_proto_set_skill(Proto* proto, int skill_id, int value) {
    if (proto == nullptr) return;

    proto->set_critter_base_stat(skill_id, value);
    logger.debug("Proto PID {} skill {} changed to {}", proto->pid, skill_id, value);
}
