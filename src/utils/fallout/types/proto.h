#ifndef CK_PROTO_H
#define CK_PROTO_H

namespace ck::utils::fallout::types {
    struct Proto {
        void *fallout_proto_ptr = nullptr;

        int source_pid = -1;
        int source_fid = -1;
        int pid = -1;

        int object_type = -1;

        const char* mod_id;
        std::string lua_tag;

        void set_fid(int fid);
        void set_sid(int sid);
        int get_sid() const;
        int get_fid() const;

        int get_msg_id() const;

        void set_item_price(int cost);
        void set_item_weight(int weight);
        void set_item_inventory_fid(int fid);
        void set_item_can_use(bool can_use);

        void set_critter_team(int team_num);
        void set_critter_ai(int ai_packet);
    };

    namespace proto {
        int type(int pid);
        Proto get(int pid);
        Proto allocate(int source_pid, int assigned_pid, const char* mod_id);
        void free(Proto &proto);
    } // namespace proto
}

#endif
