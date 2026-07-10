#ifndef CK_CONFIG_PATCH_H
#define CK_CONFIG_PATCH_H

#include <string>
#include <unordered_map>
#include "config.h"

namespace fallout {
    typedef struct Dictionary Config;
}

struct CkConfigPatch {
    std::string file_path;
    std::string section;
    std::string key;
    std::string value;
};


namespace ck {
    using ConfigPatchMap = std::unordered_map<std::string,
                           std::unordered_map<std::string,
                           std::unordered_map<std::string, std::string>>>;

    extern ConfigPatchMap g_config_patches;

    int  config_next_map_index(const std::string& file_path);
    int  config_next_area_index(const std::string& file_path);
	int config_find_entrance_by_map_name(std::string_view file_path, std::string_view section, std::string_view map_name);
	int config_count_custom_entrances(std::string_view file_path, std::string_view section);
    void config_patch_add(std::string_view file_path, std::string_view section, std::string_view key, std::string_view value);
    int  config_count_area_entrances_vfs(int area_id);
}

void ck_config_patch_apply(fallout::Config* config, const char* file_path);
void ck_config_patch_clear();

#endif
