#ifndef CK_CONFIG_PATCH_H
#define CK_CONFIG_PATCH_H

#include <string>
#include <unordered_map>

namespace fallout {
    typedef struct Dictionary Config;
}

namespace ck {
    using ConfigPatchMap = std::unordered_map<std::string,
                           std::unordered_map<std::string,
                           std::unordered_map<std::string, std::string>>>;
    extern ConfigPatchMap g_config_patches;

	int  config_find_next_free_index_vfs(const char* file_path, std::string_view prefix);
    void config_patch_add(std::string_view file_path, std::string_view section, std::string_view key, std::string_view value);
	void config_patch_apply(fallout::Config* config, const char* file_path);
	void config_patch_clear();
}

#endif
