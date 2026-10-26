#ifndef CK_IDS_H
#define CK_IDS_H

#include "ck_api.h"

namespace fallout {
	struct Object;
}

namespace ck::ids {
    enum class ObjectType : int { ITEM = 0, CRITTER = 1, SCENERY = 2, WALL = 3, TILE = 4, MISC = 5 };

    namespace script_index {
        constexpr int base = 2000;
        constexpr int limit = 16000;
    }

    // pid
    namespace proto_id {
        constexpr int base = 10000;
        constexpr int limit = 15000;
    }

    namespace frm {
        constexpr int base = 2000;
        constexpr int limit = 4095; // 0xFFF
        constexpr int transport_type = 6; // fallout::OBJ_TYPE_INTERFACE
    }

    int object_type(int pid);
    bool is_item(fallout::Object* object);
    bool is_critter(fallout::Object* object);
    bool is_scenery(fallout::Object* object);

    int script_type_for_object_type(int object_type);
    int script_type_for_object(fallout::Object* object);

    // ids registry (vectors)
    void clear_all();
    void clear_for_mod(const char* mod_id);
    void clear_for_object_type(int object_type);
    int next_script_index(const char* mod_id, int object_type);
    int next_proto_id(const char* mod_id, int object_type);
    void add_custom_script_index(int script_index, int object_type, const char* mod_id);

    int make_packed_sid(int script_type, int script_index);
    int make_packed_sid(fallout::Object* object, int script_index);

    bool is_ck_frm(int fid);
    int  art_id_from_fid(int fid);
    int  make_ck_fid(int custom_frm_id, int art_type = ck::ids::frm::transport_type);

    bool is_ck_pid(int pid);
    bool is_ck_sid(int sid);
    bool is_ck_script_index(int script_index);

    int clean_sid(int full_sid);
    int clean_pid(int full_pid);
} // namespace ck

CK_API int ck_ids_make_ck_fid(int custom_frm_id, int art_type = ck::ids::frm::transport_type);
CK_API int ck_ids_art_id_from_fid(int fid);
CK_API bool ck_is_sid_ck_custom(int sid);

#endif
