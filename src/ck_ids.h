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
constexpr int CK_PROTO_SID_START   = 10000;
constexpr int CK_PROTO_SID_LIMIT   = 11000;
constexpr int CK_SID_LIMIT         = 16000;

constexpr int CK_PID_START  = 10000;
constexpr int CK_PID_LIMIT  = 15000;

enum class ObjectType : int {
    ITEM    = 0,
    CRITTER = 1,
    SCENERY = 2,
    WALL    = 3,
    TILE    = 4,
    MISC    = 5
};

constexpr int CK_FRM_BASE              = 2000;
constexpr int CK_FRM_LIMIT             = 4095; // 0xFFF
constexpr int CK_ASSET_TRANSPORT_TYPE  = 6;    // fallout::OBJ_TYPE_INTERFACE

bool is_ck_frm(int fid);
int  art_id_from_fid(int fid);
int  make_ck_fid(int custom_frm_id, int art_type = CK_ASSET_TRANSPORT_TYPE);

bool is_ck_pid(int pid);

bool is_ck_sid(int sid);
bool is_proto_sid(int sid);
bool is_created_sid(int sid);
bool is_modified_sid(int sid);
int  lua_id_from_sid(int sid);

int make_created_sid(int lua_id);
int make_modified_sid(int lua_id);
int make_full_sid(int script_type, int custom_sid);
int make_proto_sid(int custom_sid);
int clean_sid(int full_sid);
int clean_pid(int full_pid);

int make_sid_created(fallout::Object* obj, int lua_id);
int make_sid_modified(fallout::Object* obj, int lua_id);

} // namespace ck

CK_API int ck_ids_make_ck_fid(int custom_frm_id, int art_type = ck::ids::CK_ASSET_TRANSPORT_TYPE);
CK_API int ck_ids_art_id_from_fid(int fid);

#endif
