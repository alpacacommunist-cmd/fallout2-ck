#ifndef CK_PROTO_H
#define CK_PROTO_H

namespace ck::utils::fallout::types {
    struct ItemView;
    struct CritterView;

    struct Proto {
        ItemView as_item();
        CritterView as_critter();

        void *fallout_proto_ptr = nullptr;

        int source_pid = -1;
        int source_fid = -1;
        int source_sid = -1;

        int pid = -1;
        int object_type = -1;

        const char* mod_id;
        std::string lua_tag;

        Proto() = default;
        ~Proto();

        // Generic
        void set_fid(int fid);
        void set_sid(int sid);
        int get_sid() const;
        int get_fid() const;
        int get_msg_id() const;
        int packed_pid() const;
    };

    struct ItemView {
        Proto &proto;

        void set_price(int cost);
        void set_weight(int weight);
        void set_inventory_fid(int fid);
        void set_can_use(bool can_use);
    };

    struct CritterView {
        Proto &proto;

        void set_team(int team_num);
        void set_ai(int ai_packet);
        int get_skill(int skill_id);
        int set_skill(int skill_id, int value);
        int get_base_stat(int stat_id);
        int set_base_stat(int stat_id, int value);
    };

    namespace proto {
        int type(int pid);
        int packed_pid(int pid);
        int allocate(int source_pid, int assigned_pid, Proto& proto);
    } // namespace proto
}

#endif
