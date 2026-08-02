#include "ck_ids.h"
#include "obj_types.h"
#include "scripts.h"

namespace ck::ids {

static_assert(ck::ids::object_types::ITEM    == fallout::OBJ_TYPE_ITEM,    "ITEM mismatch!");
static_assert(ck::ids::object_types::CRITTER == fallout::OBJ_TYPE_CRITTER, "CRITTER mismatch!");
static_assert(ck::ids::object_types::SCENERY == fallout::OBJ_TYPE_SCENERY, "SCENERY mismatch!");
static_assert(ck::ids::object_types::WALL    == fallout::OBJ_TYPE_WALL,    "WALL mismatch!");
static_assert(ck::ids::object_types::TILE    == fallout::OBJ_TYPE_TILE,    "TILE mismatch!");
static_assert(ck::ids::object_types::MISC    == fallout::OBJ_TYPE_MISC,    "MISC mismatch!");

int script_type_for_object(fallout::Object* object) {
    if (!object) return fallout::SCRIPT_TYPE_SYSTEM;

    int object_type = PID_TYPE(object->pid);

    switch (object_type) {
		case fallout::OBJ_TYPE_CRITTER:
            return fallout::SCRIPT_TYPE_CRITTER;
		case fallout::OBJ_TYPE_ITEM:
            return fallout::SCRIPT_TYPE_ITEM;
		case fallout::OBJ_TYPE_SCENERY:
        case fallout::OBJ_TYPE_WALL:
        case fallout::OBJ_TYPE_TILE:
        case fallout::OBJ_TYPE_MISC:
            return fallout::SCRIPT_TYPE_SPATIAL;
        default:
            return fallout::SCRIPT_TYPE_SYSTEM;
    }
}

bool is_ck_frm(int fid) {
    int object_type = (fid >> 24) & 0x0F;
    int frm_id      = fid & 0xFFF;
    return (object_type == CK_ASSET_TRANSPORT_TYPE && frm_id >= CK_FRM_BASE && frm_id <= CK_FRM_LIMIT);
}

int frm_id_from_fid(int fid) {
    return fid & 0xFFF;
}

int make_ck_fid(int custom_frm_id) {
    return (CK_ASSET_TRANSPORT_TYPE << 24) | (custom_frm_id & 0xFFF);
}

int clean_sid(int full_sid) {
    return full_sid & 0x00FFFFFF;
}

bool is_ck_sid(int sid) {
	int clean = clean_sid(sid);
    return (clean >= CK_SID_BASE && clean <= CK_SID_LIMIT);
}

bool is_created_sid(int sid) {
	int clean = clean_sid(sid);
    return (clean >= CK_CREATED_START && clean <= CK_CREATED_LIMIT);
}

bool is_modified_sid(int sid) {
	int clean = clean_sid(sid);
    return (clean >= CK_MODIFIED_START && clean <= CK_MODIFIED_LIMIT);
}

int lua_id_from_sid(int sid) {
	int clean = clean_sid(sid);

    if (is_modified_sid(sid))     return clean - CK_MODIFIED_START;
    if (is_created_sid(sid))      return clean - CK_CREATED_START;
    if (is_ck_sid(sid))           return clean - CK_SID_BASE;
    return -1;
}

int make_created_sid(int lua_id) {
    return CK_CREATED_START + lua_id;
}

int make_modified_sid(int lua_id) {
    return CK_MODIFIED_START + lua_id;
}

int make_full_sid(int script_type, int custom_sid) {
    return (script_type << 24) | (custom_sid & 0x00FFFFFF);
}

int make_sid_created(fallout::Object* obj, int lua_id) {
    int script_type = script_type_for_object(obj);
    int custom_sid  = make_created_sid(lua_id);

    return make_full_sid(script_type, custom_sid);
}

int make_sid_modified(fallout::Object* obj, int lua_id) {
    int script_type = script_type_for_object(obj);
    int custom_sid  = make_modified_sid(lua_id);

    return make_full_sid(script_type, custom_sid);
}

} // namespace ck
