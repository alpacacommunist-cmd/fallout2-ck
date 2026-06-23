#include "ck_config_patch.h"
#include "db.h"

#include <vector>
#include <iostream>
#include <string_view>
#include <charconv>

#include "ck_log.h"
static const Logger log("CK Config Patch");

namespace fallout {
    bool configSetString(Config*, const char*, const char*, const char*);
}

static std::vector<CkConfigPatch> gPatches;

static int ck_find_last_index_vfs(const char* file_path, std::string_view prefix) {
	fallout::File* f = fallout::fileOpen(file_path, "rt");
    if (f == nullptr) {
        log.error("[CK Config Patch] VFS cannot open: {}", file_path);
        return -1;
    }

    int last_index = -1;
    char line[1024];

	std::string search_prefix = "[";
    search_prefix.append(prefix).append(" ");

    while (fileReadString(line, sizeof(line), f) != nullptr) {
        std::string_view s(line);
        // trim \r\n
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) { s.remove_suffix(1); }

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

int ck_config_next_map_index(const std::string& file_path) {
    int last = ck_find_last_index_vfs(file_path.c_str(), "Map");
    log.info("Last map index: {}", last);
    return last == -1 ? 0 : last + 1;
}

int ck_config_next_area_index(const std::string& file_path) {
    int last = ck_find_last_index_vfs(file_path.c_str(), "Area");
    log.info("Last area index: {}", last);
    return last == -1 ? 0 : last + 1;
}

void ck_config_patch_add(std::string_view file_path, std::string_view section,
                         std::string_view key, std::string_view value) {

	std::string normalized_path(file_path);
	for (char& c : normalized_path) if (c == '\\') c = '/';

    gPatches.push_back({ normalized_path, std::string(section), std::string(key), std::string(value) });
	log.info("Registered: [{}] {} = {} ({})", section, key, value, normalized_path);
}

void ck_config_patch_apply(fallout::Config* config, const char* file_path) {
    if (config == nullptr || file_path == nullptr) return;

	std::string check_path = file_path;
	for (char& c : check_path) if (c == '\\') c = '/';

    int applied = 0;
    for (const auto& patch : gPatches) {
        // quick compare (no separation/registry)
        // "data\\city.txt" == "data/city.txt"

        if (patch.file_path == check_path) {
            fallout::configSetString(config, patch.section.c_str(), patch.key.c_str(), patch.value.c_str());
            applied++;
        }
    }

    if (applied > 0) {
		log.info("Applied {} patches to: {}", applied, file_path);
    }
}

void ck_config_patch_clear() {
    gPatches.clear();
    log.info("Cleared all patches.");
}
