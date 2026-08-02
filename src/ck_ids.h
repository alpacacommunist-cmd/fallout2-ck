#ifndef CK_IDS_H
#define CK_IDS_H

#include "ck_api.h"

namespace fallout {
	struct Object;
}

namespace ck::ids {

constexpr int CK_SID_BASE          = 2000;
constexpr int CK_CREATED_START     = 2000;
constexpr int CK_CREATED_LIMIT     = 6000;
constexpr int CK_MODIFIED_START    = 6000;
constexpr int CK_MODIFIED_LIMIT    = 10000;
constexpr int CK_SID_LIMIT         = 15000;

namespace object_types {
	constexpr int ITEM       = 0;
    constexpr int CRITTER    = 1;
    constexpr int SCENERY    = 2;
    constexpr int WALL       = 3;
    constexpr int TILE       = 4;
    constexpr int MISC       = 5;
}

constexpr int CK_FRM_BASE              = 1000;
constexpr int CK_FRM_LIMIT             = 4095; // 0xFFF
constexpr int CK_ASSET_TRANSPORT_TYPE  = 6;    // fallout::OBJ_TYPE_INTERFACE

bool is_ck_frm(int fid);
int  frm_id_from_fid(int fid);
int  make_ck_fid(int custom_frm_id);

bool is_ck_sid(int sid);
bool is_created_sid(int sid);
bool is_modified_sid(int sid);
int  lua_id_from_sid(int sid);

int make_created_sid(int lua_id);
int make_modified_sid(int lua_id);
int make_full_sid(int script_type, int custom_sid);
int clean_sid(int full_sid);

int make_sid_created(fallout::Object* obj, int lua_id);
int make_sid_modified(fallout::Object* obj, int lua_id);

} // namespace ck

CK_API int ck_ids_make_ck_fid(int custom_frm_id);

#endif
