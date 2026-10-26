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
        constexpr int created_base = 2000;
        constexpr int created_limit = 6000;
        constexpr int modified_base = 6000;
        constexpr int modified_limit = 10000;
        constexpr int proto_base = 10000;
        constexpr int proto_limit = 11000;
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

bool is_item(fallout::Object* object);
bool is_critter(fallout::Object* object);
bool is_scenery(fallout::Object* object);

int script_type_for_object_type(int object_type);
int script_type_for_object(fallout::Object* object);

bool is_ck_frm(int fid);
int  art_id_from_fid(int fid);
int  make_ck_fid(int custom_frm_id, int art_type = ck::ids::frm::transport_type);

bool is_ck_pid(int pid);

bool is_ck_sid(int sid);
bool is_ck_proto_sid(int sid);
bool is_created_sid(int sid);
bool is_modified_sid(int sid);
int  lua_id_from_sid(int sid);

int make_created_sid(int lua_id);
int make_modified_sid(int lua_id);
int make_proto_sid(int custom_sid, int object_type);
int clean_sid(int full_sid);
int clean_pid(int full_pid);

int make_sid_created(fallout::Object* obj, int lua_id);
int make_sid_modified(fallout::Object* obj, int lua_id);

} // namespace ck

CK_API int ck_ids_make_ck_fid(int custom_frm_id, int art_type = ck::ids::frm::transport_type);
CK_API int ck_ids_art_id_from_fid(int fid);
CK_API bool ck_is_sid_ck_custom(int sid);

#endif
