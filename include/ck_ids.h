#ifndef CK_IDS_H
#define CK_IDS_H

#include "obj_types.h"
#include "scripts.h"

static const int CK_SID_BASE = 2000;
static const int CK_SID_LIMIT = 10000;

namespace ck {

// ---- SID helpers ----
inline int  make_sid(int luaId)       { return CK_SID_BASE + luaId; }
inline bool is_ck_sid(int sid)        { return (sid >= CK_SID_BASE && sid <= CK_SID_LIMIT); }
inline int  lua_id_from_sid(int sid)  { return sid - CK_SID_BASE; }
inline int  make_full_sid(int scriptType, int customSid) {
    return (scriptType << 24) | (customSid & 0x00FFFFFF);
}

inline int clean_sid(int fullSid) {
    return fullSid & 0x00FFFFFF;
}

// ---- PID helpers ----
inline int make_pid(int type, int id) { return (type << 24) | (id & 0x00FFFFFF); }
inline int pid_type(int pid)          { return PID_TYPE(pid); }
inline int pid_id(int pid)            { return pid & 0x00FFFFFF; }

// ---- FID helpers ----
inline int make_fid(int type, int artId, int rotation = 0, int weaponCode = 0) {
    return ((rotation & 0x7) << 28) |
           ((type & 0xF) << 24) |
           ((weaponCode & 0xF) << 12) |
           (artId & 0xFFF);
}
inline int fid_type(int fid)     { return FID_TYPE(fid); }
inline int fid_art_id(int fid)   { return fid & 0x00000FFF; }
inline int fid_rotation(int fid) { return FID_ROTATION(fid); }
inline int fid_weapon(int fid)   { return FID_WEAPON_CODE(fid); }

} // namespace ck

#endif
