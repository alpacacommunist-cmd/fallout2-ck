#ifndef CK_TYPES_PROTO_H
#define CK_TYPES_PROTO_H

#include <cstdint>
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

        // Procs
        std::array<uint8_t, 28> allowed_procs{};

        void set_allowed_procs(const uint8_t* procs) {
            bool any = false;
            for (size_t index = 0; index < allowed_procs.size(); ++index) {
                allowed_procs[index] = procs[index] ? 1 : 0;

                if (allowed_procs[index]) any = true;
            }

            has_custom_procs = any;
        }

        bool is_proc_allowed(int proc) const {
            if (proc < 0 || proc >= static_cast<int>(allowed_procs.size())) return false;
            return allowed_procs[proc] != 0;
        }

        bool has_custom_procs = false;

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
        int packed_pid(int pid, int object_type);
        int allocate(int source_pid, int assigned_pid, Proto& proto);
    } // namespace proto
}

#endif
