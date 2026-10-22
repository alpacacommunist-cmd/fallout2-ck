#ifndef CK_PROTO_H
#define CK_PROTO_H

namespace ck::utils::fallout::types {
    struct Proto {
        void *fallout_proto_ptr = nullptr;

        int source_pid = -1;
        int source_fid = -1;
        int source_sid = -1;

        int pid = -1;
        int object_type = -1;

        const char* mod_id;
        std::string lua_tag;

        // Generic
        void set_fid(int fid);
        void set_sid(int sid);
        int get_sid() const;
        int get_fid() const;
        int get_msg_id() const;
        int packed_pid() const;

        // Item
        void set_item_price(int cost);
        void set_item_weight(int weight);
        void set_item_inventory_fid(int fid);
        void set_item_can_use(bool can_use);

        // Critter
        void set_critter_team(int team_num);
        void set_critter_ai(int ai_packet);
        int get_critter_skill(int skill_id);
        int set_critter_skill(int skill_id, int value);
        int get_critter_base_stat(int stat_id);
        int set_critter_base_stat(int stat_id, int value);
    };

    namespace proto {
        int type(int pid);
        int packed_pid(int pid);
        Proto get(int pid);
        Proto allocate(int source_pid, int assigned_pid, const char* mod_id);
        void free(Proto &proto);
    } // namespace proto
}

#endif
