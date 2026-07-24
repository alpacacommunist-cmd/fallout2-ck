#ifndef CK_IDS_H
#define CK_IDS_H

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

#endif
