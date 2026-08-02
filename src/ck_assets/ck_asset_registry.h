#ifndef CK_ASSET_REGISTRY_H
#define CK_ASSET_REGISTRY_H

#include "ck_api.h"

namespace ck::assets {
    int         register_path(const char* path);
    const char* get_custom_mod_file_path(int frm_id);

    void clear();
    bool is_ck_frm(int fid);
}

CK_API int ck_assets_register_path(const char* path);

#endif
