#include "utils.h"
#include "ck_ids.h"

#include "ck_messages/ck_messages.h"
#include "ck_proto_registry.h"

#include "log.h"
static constexpr logging::Context logger{"ck_item_proto.cc"};

namespace fallout {
    int combat_ai_packet_num_by_name(const char* name);
}

namespace ck::proto::registry::critter {
    using namespace ck::utils::fallout;

    int register_critter_prototype(int source_pid, const char* lua_tag, const ItemProtoFFI& ffi_data) {
        int assigned_pid = registry::build_generic_prototype(source_pid, lua_tag);
        if (assigned_pid == -1) return -1;

        types::Proto* proto = ck::proto::registry::get(assigned_pid);

        int msg_name_id = proto->get_msg_id();
        if (!utils::is_blank(ffi_data.name)) {
            ck::messages_add_string("pro_crit.msg", msg_name_id, ffi_data.name);
        }

        if (!utils::is_blank(ffi_data.description)) {
            ck::messages_add_string("pro_crit.msg", msg_name_id + 1, ffi_data.description);
        }

        return proto->pid;
    }
}
