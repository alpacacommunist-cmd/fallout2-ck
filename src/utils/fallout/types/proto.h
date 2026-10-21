#ifndef CK_PROTO_H
#define CK_PROTO_H

namespace ck::utils::fallout::types {
    struct Proto {
        void *fallout_proto_ptr = nullptr;

        int source_pid = 0;
        int pid = 0; // pid
        int object_type = 0;

        void set_fid(int fid);
        void set_sid(int sid);

        void set_item_cost(int cost);
        void set_item_weight(int weight);
        void set_item_inventory_fid(int fid);
        void set_item_can_use(bool can_use);

        void set_critter_team(int team_num);
        void set_critter_ai(int ai_packet);
    };

    namespace proto {
        Proto get(int pid);
        Proto allocate(int source_pid, int assigned_pid, int object_type);
        void free(Proto &proto);
    } // namespace proto
}

#endif
