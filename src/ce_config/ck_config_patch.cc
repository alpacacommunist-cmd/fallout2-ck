#include "ck_config_patch.h"
#include "db.h"

#include <vector>
#include <iostream>
#include <string_view>
#include <charconv>

namespace fallout {
    bool configSetString(Config*, const char*, const char*, const char*);
}

static std::vector<CkConfigPatch> gPatches;

static int ck_find_last_index_vfs(const char* filePath, std::string_view prefix) {
	fallout::File* f = fallout::fileOpen(filePath, "rt");
    if (f == nullptr) {
        std::cerr << "[CK Config Patch] VFS cannot open: " << filePath << std::endl;
        return -1;
    }

    int last_index = -1;
    char line[1024];

	std::string search_prefix = "[";
    search_prefix.append(prefix).append(" ");

    while (fileReadString(line, sizeof(line), f) != nullptr) {
        std::string_view s(line);
        // trim \r\n
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) { s.remove_prefix(1); }

        if (s.starts_with(search_prefix)) {
            size_t start = search_prefix.size();
            size_t end   = s.find(']', start);

            if (end != std::string_view::npos) {
				std::string_view sub_view = s.substr(start, end - start);

				int  idx = 0;
                auto [ptr, ec] = std::from_chars(sub_view.data(), sub_view.data() + sub_view.size(), idx);
				if (ec == std::errc()) {
					if (idx > last_index) last_index = idx;
				}
            }
        }
    }

    fileClose(f);
    return last_index;
}

int ck_config_next_map_index(const std::string& filePath) {
    int last = ck_find_last_index_vfs(filePath.c_str(), "Map");
    std::cout << "[CK Config Patch] Last map index: " << last << std::endl;
    return last == -1 ? 0 : last + 1;
}

int ck_config_next_area_index(const std::string& filePath) {
    int last = ck_find_last_index_vfs(filePath.c_str(), "Area");
    std::cout << "[CK Config Patch] Last area index: " << last << std::endl;
    return last == -1 ? 0 : last + 1;
}

void ck_config_patch_add(const std::string& filePath, const std::string& section,
                         const std::string& key, const std::string& value) {

	std::string normalized_path = filePath;
	for (char& c : normalized_path) if (c == '\\') c = '/';

    gPatches.push_back({ normalized_path, section, key, value });
    std::cout << "[CK Config Patch] Registered: ["
              << section << "] " << key << " = " << value
              << " (" << normalized_path << ")" << std::endl;
}

void ck_config_patch_apply(fallout::Config* config, const char* filePath) {
    if (config == nullptr || filePath == nullptr) return;

	std::string check_path = filePath;
	for (char& c : check_path) if (c == '\\') c = '/';

    int applied = 0;
    for (const auto& patch : gPatches) {
        // quick compare (no separation/registry)
        // "data\\city.txt" == "data/city.txt"

        if (patch.filePath == check_path) {
            fallout::configSetString(config, patch.section.c_str(), patch.key.c_str(), patch.value.c_str());
            applied++;
        }
    }

    if (applied > 0) {
        std::cout << "[CK Config Patch] Applied " << applied
                  << " patches to: " << filePath << std::endl;
    }
}

void ck_config_patch_clear() {
    gPatches.clear();
    std::cout << "[CK Config Patch] Cleared all patches." << std::endl;
}
