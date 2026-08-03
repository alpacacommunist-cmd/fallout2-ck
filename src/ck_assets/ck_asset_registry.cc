#include "ck_ids.h"
#include "ck_assets/ck_asset_registry.h"

#include <unordered_map>
#include <string>

#include "ck_log.h"
static const Logger log("CK Asset Registry");

namespace ck::assets {
    // [frm_id] -> "/mods/my_mod/art/..."
    static std::unordered_map<int, std::string> g_custom_paths;
    static int g_next_free_frm_id = ck::ids::CK_FRM_BASE;

    int register_path(const char* path) {
        if (!path || g_next_free_frm_id > ck::ids::CK_FRM_LIMIT) {
            return -1;
        }

        int assigned_id = g_next_free_frm_id++;
        g_custom_paths[assigned_id] = path;

        log.debug("assigned asset path: {} to id: {}", path, assigned_id);

        return assigned_id;
    }

    const char* get_custom_mod_file_path(int frm_id) {
        auto it = g_custom_paths.find(frm_id);
        if (it != g_custom_paths.end()) {
            return it->second.c_str();
        }
        return nullptr;
    }

    void clear() {
        g_custom_paths.clear();
        g_next_free_frm_id = ck::ids::CK_FRM_BASE;

        log.debug("Registry cleared, next_frm_id: {}", g_next_free_frm_id);
    }

    bool is_ck_frm(int fid) {
        return ck::ids::is_ck_frm(fid);
    }
}

int ck_assets_register_path(const char* path) {
    return ck::assets::register_path(path);
}
