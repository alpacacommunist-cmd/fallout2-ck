#include "utils/fallout/types/proto.h"
#include "fallout2-ce/proto_types.h"

#include <memory>
#include <cstring>

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
        0,
        0,
        0,
        0,
        0,
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

    void Proto::set_fid(int fid) {
        proto::as_generic(fallout_proto_ptr)->fid = fid;
    }

    void Proto::set_sid(int sid) {
        proto::as_generic(fallout_proto_ptr)->sid = sid;
    }

    int Proto::get_sid() const {
        return proto::as_generic(fallout_proto_ptr)->sid;
    }

    void Proto::set_item_cost(int cost) {
        proto::as_item(fallout_proto_ptr)->cost = cost;
    }

    void Proto::set_item_weight(int weight) {
        proto::as_item(fallout_proto_ptr)->weight = weight;
    }

    void Proto::set_item_inventory_fid(int fid) {
        proto::as_item(fallout_proto_ptr)->inventoryFid = fid;
    }

    void Proto::set_item_can_use(bool can_use) {
        auto* item = proto::as_item(fallout_proto_ptr);
        if (can_use) {
            item->extendedFlags |= ::fallout::ProtoExtendedFlags::PROTO_EXT_FLAG_CAN_USE;
        } else {
            item->extendedFlags = static_cast<::fallout::ProtoExtendedFlags>(
                item->extendedFlags & ~::fallout::ProtoExtendedFlags::PROTO_EXT_FLAG_CAN_USE);
        }
    }

    void Proto::set_critter_team(int team_num) {
        proto::as_critter(fallout_proto_ptr)->team = team_num;
    }

    void Proto::set_critter_ai(int ai_packet) {
        proto::as_critter(fallout_proto_ptr)->aiPacket = ai_packet;
    }

    namespace proto {
        struct MallocDeleter {
            void operator()(void *ptr) const {
                std::free(ptr);
            }
        };

        using UniqueProtoPtr = std::unique_ptr<Proto, MallocDeleter>;

        Proto get(int pid) {
            ::fallout::ProtoId proto_id{pid};
            ::fallout::Proto *generic_proto = nullptr;

            if (::fallout::protoGetProto(proto_id, &generic_proto) == 0 && generic_proto) {
                return Proto{generic_proto, pid, pid, proto_id.objectType()};
            }

            return Proto{};
        }

        Proto allocate(int source_pid, int assigned_pid) {
            ::fallout::ProtoId proto_id { source_pid };
            ::fallout::Proto *source_proto = nullptr;

            ::fallout::protoGetProto(proto_id, &source_proto);
            if (!source_proto) return Proto{};

            int object_type = proto_id.objectType();

            size_t size = ::fallout::_proto_sizes[object_type];
            void *mem = std::malloc(size);
            if (!mem) return Proto{};

            std::memcpy(mem, source_proto, size);

            auto *fallout_proto = as_generic(mem);
            fallout_proto->pid = assigned_pid;
            fallout_proto->messageId = assigned_pid * 100;

            return Proto{mem, source_pid, assigned_pid, object_type};
        }

        void free(Proto &proto) {
            if (proto.fallout_proto_ptr) {
                std::free(proto.fallout_proto_ptr);
                proto.fallout_proto_ptr = nullptr;
            }
        }
    } // namespace proto
} // namespace ck::utils::fallout::types
