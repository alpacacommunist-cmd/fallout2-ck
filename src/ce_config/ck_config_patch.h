#ifndef CK_CONFIG_PATCH_H
#define CK_CONFIG_PATCH_H

#include <string>
#include <vector>
#include "config.h"

namespace fallout {
    typedef struct Dictionary Config;
}

struct CkConfigPatch {
    std::string filePath;
    std::string section;
    std::string key;
    std::string value;
};

int ck_config_next_map_index(const std::string& mapsFilePath);
int ck_config_next_area_index(const std::string& cityFilePath);

void ck_config_patch_add(std::string_view filePath,
                         std::string_view section,
                         std::string_view key,
                         std::string_view value);

void ck_config_patch_apply(fallout::Config* config, const char* filePath);
void ck_config_patch_clear();

#endif
