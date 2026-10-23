#include "utils/fallout/types/proto.h"
#include "fallout2-ce/proto_types.h"
#include <cstring>

#include "log.h"
static constexpr logging::Context logger{"types/proto.cc"};

namespace fallout {
    // proto.cc:
    // 0x51C340 proto_sizes
    // static const size_t _proto_sizes[OBJ_TYPE_COUNT] = { ...
    const size_t _proto_sizes[11] = {
        sizeof(ItemProto),    // 0x84
        sizeof(CritterProto), // 0x1A0
        sizeof(SceneryProto), // 0x38
        sizeof(WallProto),    // 0x24
        sizeof(TileProto),    // 0x1C
        sizeof(MiscProto),    // 0x1C
        0, 0, 0, 0, 0,
    };

    // proto.cc::
    // proto_ptr
    // 0x4A2108 proto_ptr
    int protoGetProto(const ProtoId &protoId, Proto **protoPtr);
} // namespace fallout

namespace ck::utils::fallout::types {
    namespace proto {
        [[nodiscard]] static inline ::fallout::ItemProto *as_item(void *ptr) {
            return reinterpret_cast<::fallout::ItemProto *>(ptr);
        }

        [[nodiscard]] static inline ::fallout::CritterProto *as_critter(void *ptr) {
            return reinterpret_cast<::fallout::CritterProto *>(ptr);
        }

        [[nodiscard]] static inline ::fallout::Proto *as_generic(void *ptr) {
            return static_cast<::fallout::Proto *>(ptr);
        }
    }

    ItemView Proto::as_item() { return ItemView{*this}; }
    CritterView Proto::as_critter() { return CritterView{*this}; }

    Proto::~Proto() {
        if (fallout_proto_ptr) {
            std::free(fallout_proto_ptr);
            fallout_proto_ptr = nullptr;
        }
    }

    // Generic
    int Proto::get_msg_id() const {
        return proto::as_generic(fallout_proto_ptr)->messageId;
    }

    int Proto::get_sid() const {
        return proto::as_generic(fallout_proto_ptr)->sid;
    }

    int Proto::get_fid() const {
        return proto::as_generic(fallout_proto_ptr)->fid;
    }

    void Proto::set_sid(int sid) {
        proto::as_generic(fallout_proto_ptr)->sid = sid;
    }

    void Proto::set_fid(int fid) {
        proto::as_generic(fallout_proto_ptr)->fid = fid;
    }

    int Proto::packed_pid() const {
        return (object_type << 24) | (pid);
    }

    // Item
    void ItemView::set_price(int price) {
        proto::as_item(proto.fallout_proto_ptr)->cost = price;
    }

    void ItemView::set_weight(int weight) {
        proto::as_item(proto.fallout_proto_ptr)->weight = weight;
    }

    void ItemView::set_inventory_fid(int fid) {
        proto::as_item(proto.fallout_proto_ptr)->inventoryFid = fid;
    }

    void ItemView::set_can_use(bool can_use) {
        auto* item = proto::as_item(proto.fallout_proto_ptr);
        if (can_use) {
            item->extendedFlags |= ::fallout::ProtoExtendedFlags::PROTO_EXT_FLAG_CAN_USE;
        } else {
            item->extendedFlags = static_cast<::fallout::ProtoExtendedFlags>(
                item->extendedFlags & ~::fallout::ProtoExtendedFlags::PROTO_EXT_FLAG_CAN_USE);
        }
    }

    // Critter
    void CritterView::set_team(int team_num) {
        proto::as_critter(proto.fallout_proto_ptr)->team = team_num;
    }

    void CritterView::set_ai(int ai_packet) {
        proto::as_critter(proto.fallout_proto_ptr)->aiPacket = ai_packet;
    }

    int CritterView::get_skill(int skill_id) {
        return proto::as_critter(proto.fallout_proto_ptr)->data.skills[skill_id];
    }

    int CritterView::set_skill(int skill_id, int value) {
        return proto::as_critter(proto.fallout_proto_ptr)->data.skills[skill_id] = value;
    }

    int CritterView::get_base_stat(int stat_id) {
        return proto::as_critter(proto.fallout_proto_ptr)->data.baseStats[stat_id];
    }

    int CritterView::set_base_stat(int stat_id, int value) {
        return proto::as_critter(proto.fallout_proto_ptr)->data.baseStats[stat_id] = value;
    }

    namespace proto {
        int type(int pid) {
            ::fallout::ProtoId proto_id{pid};
            return static_cast<int>(proto_id.objectType());
        }

        int packed_pid(int pid, int object_type) {
            return (object_type << 24) | (pid);
        }

        int allocate(int source_pid, int assigned_pid, Proto& proto) {
            ::fallout::ProtoId proto_id { source_pid };
            ::fallout::Proto *source_proto = nullptr;

            ::fallout::protoGetProto(proto_id, &source_proto);
            if (!source_proto) return -1;

            int object_type = proto_id.objectType();

            size_t size = ::fallout::_proto_sizes[object_type];
            void *memory = std::malloc(size);
            if (!memory) return -1;

            std::memcpy(memory, source_proto, size);

            ::fallout::Proto* fallout_proto = as_generic(memory);
            fallout_proto->pid = types::proto::packed_pid(assigned_pid, object_type);
            fallout_proto->messageId = assigned_pid * 100;

            proto.fallout_proto_ptr = fallout_proto;
            proto.source_pid = source_proto->pid;
            proto.source_fid = source_proto->fid;
            proto.source_sid = source_proto->sid;
            proto.pid = assigned_pid;
            proto.object_type = proto_id.objectType();

            return 0;
        }
    } // namespace proto
} // namespace ck::utils::fallout::types

