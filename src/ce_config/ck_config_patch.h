#ifndef CK_CONFIG_PATCH_H
#define CK_CONFIG_PATCH_H

#include "ck_api.h"
#include <string>
#include <unordered_map>

namespace fallout {
    typedef struct Dictionary Config;
    typedef Dictionary ConfigSection;
}

namespace ck {
    using ConfigPatchMap = std::unordered_map<std::string,
                           std::unordered_map<std::string,
                           std::unordered_map<std::string,
                           std::unordered_map<std::string, std::string>>>>;

    extern ConfigPatchMap g_config_patches;

	std::string normalize_config_path(std::string_view path);

    bool apply_worldmap_patches();
    void config_patch_add(std::string_view mod_id, std::string_view file_path, std::string_view section,
			std::string_view key, std::string_view value);
	void config_patch_apply(fallout::Config* config, const char* file_path);

	void config_patch_clear();
	void clear_config_patches_for_mod(const char* mod_id);
}

CK_API void ck_config_clear_mod_patches(const char* mod_id);

#endif
