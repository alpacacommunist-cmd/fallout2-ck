#include <algorithm>

#include "ck_ids.h"
#include "utils/fallout/types/proto.h"

#include "fallout2-ce/obj_types.h"

#include "log.h"
static constexpr logging::Context logger{"src/utils/ck_ids.cc"};

namespace fallout {
    typedef enum ScriptType {
        SCRIPT_TYPE_SYSTEM, // s_system
        SCRIPT_TYPE_SPATIAL, // s_spatial
        SCRIPT_TYPE_TIMED, // s_time
        SCRIPT_TYPE_ITEM, // s_item
        SCRIPT_TYPE_CRITTER, // s_critter
        SCRIPT_TYPE_COUNT,
    } ScriptType;
}

namespace ck_proto = ck::utils::fallout::types::proto;

namespace ck::ids {
    static_assert(static_cast<int>(ObjectType::ITEM) == fallout::OBJ_TYPE_ITEM, "ITEM mismatch!");
    static_assert(static_cast<int>(ObjectType::CRITTER) == fallout::OBJ_TYPE_CRITTER, "CRITTER mismatch!");
    static_assert(static_cast<int>(ObjectType::SCENERY) == fallout::OBJ_TYPE_SCENERY, "SCENERY mismatch!");
    static_assert(static_cast<int>(ObjectType::WALL) == fallout::OBJ_TYPE_WALL, "WALL mismatch!");
    static_assert(static_cast<int>(ObjectType::TILE) == fallout::OBJ_TYPE_TILE, "TILE mismatch!");
    static_assert(static_cast<int>(ObjectType::MISC) == fallout::OBJ_TYPE_MISC, "MISC mismatch!");

    int object_type(int pid) {
        return ck_proto::type(pid);
    }

    bool is_item(fallout::Object *object) {
        return ck_proto::type(object->pid) == static_cast<int>(ObjectType::ITEM);
    }

    bool is_critter(fallout::Object *object) {
        return ck_proto::type(object->pid) == static_cast<int>(ObjectType::CRITTER);
    }

    bool is_scenery(fallout::Object *object) {
        return ck_proto::type(object->pid) == static_cast<int>(ObjectType::SCENERY);
    }

    int script_type_for_object_type(int object_type) {
        switch (object_type) {
        case fallout::OBJ_TYPE_CRITTER: return fallout::SCRIPT_TYPE_CRITTER;
        case fallout::OBJ_TYPE_ITEM:    return fallout::SCRIPT_TYPE_ITEM;
        case fallout::OBJ_TYPE_SCENERY:
        case fallout::OBJ_TYPE_WALL:
        case fallout::OBJ_TYPE_TILE:
        case fallout::OBJ_TYPE_MISC:    return fallout::SCRIPT_TYPE_SPATIAL;
        default:                        return fallout::SCRIPT_TYPE_SYSTEM;
        }
    }

    int script_type_for_object(fallout::Object *object) {
        if (!object) return fallout::SCRIPT_TYPE_SYSTEM;
        return script_type_for_object_type(ck_proto::type(object->pid));
    }

    namespace {
        struct AllocatedId {
            int id;
            int object_type;
            const char* mod_id;
        };

        std::vector<AllocatedId> script_indexes;
        std::vector<AllocatedId> proto_ids;

        bool script_index_occupied(int id) {
            return std::ranges::any_of(script_indexes, [id](const auto &item) { return item.id == id; });
        }

        bool proto_id_occupied(int id) {
            return std::ranges::any_of(proto_ids, [id](const auto &item) { return item.id == id; });
        }
    } // namespace

    void clear_all() {
        script_indexes.clear();
        proto_ids.clear();
    }

    void clear_for_mod(const char* mod_id) {
        std::erase_if(script_indexes, [&mod_id](const auto& item) { return item.mod_id == mod_id; });
        std::erase_if(proto_ids, [&mod_id](const auto& item) { return item.mod_id == mod_id; });
    }

    void clear_for_object_type(int object_type) {
        std::erase_if(script_indexes, [&object_type](const auto& item) { return item.object_type == object_type; });
        std::erase_if(proto_ids, [&object_type](const auto& item) { return item.object_type == object_type; });
    }

    int next_script_index(const char* mod_id, int object_type) {
        for (int index = script_index::base; index < script_index::limit; ++index) {
            bool occupied = script_index_occupied(index);

            if (!occupied) {
                script_indexes.push_back({index, object_type, mod_id});
                return index;
            }
        }

        return -1;
    }

    int next_proto_id(const char* mod_id, int object_type) {
        for (int pid = proto_id::base; pid < proto_id::limit; ++pid) {
            bool occupied = proto_id_occupied(pid);

            if (!occupied) {
                proto_ids.push_back({pid, object_type, mod_id});
                return pid;
            }
        }
        return -1;
    }

    void add_custom_script_index(int script_index, int object_type, const char* mod_id) {
        script_indexes.push_back({script_index, object_type, mod_id});
    }

    int make_packed_sid(int script_type, int script_index) {
        return (script_type << 24) | (script_index & 0x00FFFFFF);
    }

    int make_packed_sid(fallout::Object* object, int script_index) {
        return (script_type_for_object(object) << 24) | (script_index & 0x00FFFFFF);
    }

    bool is_ck_frm(int fid) {
        if (fid == -1) return false;

        int frm_id = fid & 0xFFF;
        return frm_id >= frm::base && frm_id <= frm::limit;
    }

    int art_id_from_fid(int fid) {
        return fid & 0xFFF;
    }

    int make_ck_fid(int custom_frm_id, int art_type) {
        return ((art_type & 0xF) << 24) | (custom_frm_id & 0xFFF);
    }

    int clean_sid(int packed_sid) {
        if ((packed_sid) == -1) return -1;
        return packed_sid & 0x00FFFFFF;
    }

    int clean_pid(int packed_pid) {
        if ((packed_pid) == -1) return -1;
        return packed_pid & 0x00FFFFFF;
    }

    bool is_ck_pid(int pid) {
        int clean = clean_pid(pid);
        return (clean >= proto_id::base && clean < proto_id::limit);
    }

    bool is_ck_sid(int sid) {
        int clean = clean_sid(sid);
        return (clean >= script_index::base && clean <= script_index::limit);
    }

    bool is_ck_script_index(int script_index) {
        return script_index_occupied(script_index);
    }
} // namespace ck::ids

int ck_ids_make_ck_fid(int custom_frm_id, int art_type) {
    logger.info("ART_TYPE: {}", art_type);
    return ck::ids::make_ck_fid(custom_frm_id, art_type);
}

int ck_ids_art_id_from_fid(int fid) {
    return ck::ids::art_id_from_fid(fid);
}

bool ck_is_sid_ck_custom(int sid) {
    return ck::ids::is_ck_sid(sid);
}
