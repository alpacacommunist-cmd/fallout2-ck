#include "ce_config/ck_config_patch.h"
#include "db.h"

#include <algorithm>
#include <charconv>

#include "ck_log.h"
static const Logger log("CK Config Patch");

namespace fallout {
    bool configSetString(Config*, const char*, const char*, const char*);
}

static std::string normalize_config_path(std::string_view path) {
    std::string result(path);
    for (char& c : result) {
        if (c == '\\') c = '/';
    }
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

namespace ck {
	// : [file][section][key] = value
	ConfigPatchMap g_config_patches;

    void config_patch_add(std::string_view file_path, std::string_view section,
			std::string_view key, std::string_view value) {

        std::string path_norm = normalize_config_path(file_path);
        std::string sec_str(section);
        std::string key_str(key);
        std::string val_str(value);

        if (g_config_patches[path_norm][sec_str].contains(key_str)) {
            log.warn("Config key override! [{}] {} -> changed from '{}' to '{}'",
                     section, key, g_config_patches[path_norm][sec_str][key_str], value);
        } else {
            log.info("Registered patch: [{}] {} = {} ({})", section, key, value, path_norm);
        }

        g_config_patches[path_norm][sec_str][key_str] = val_str;
    }

	int config_find_next_free_index_vfs(const char* file_path, std::string_view prefix) {
        fallout::File* f = fallout::fileOpen(file_path, "rt");
        if (f == nullptr) {
            log.error("VFS cannot open configuration file for index scanning: {}", file_path);
            return 0;
        }

        int last_index = -1;
        char line[1024];

        std::string search_prefix = "[";
        search_prefix.append(prefix).append(" ");

        while (fileReadString(line, sizeof(line), f) != nullptr) {
            std::string_view s(line);

            while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
                s.remove_suffix(1);
            }

            while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
                s.remove_prefix(1);
            }

            if (s.empty()) continue;

            if (s.starts_with(search_prefix)) {
                size_t start = search_prefix.size();
                size_t end   = s.find(']', start);

                if (end != std::string_view::npos) {
                    std::string_view sub_view = s.substr(start, end - start);
                    int idx = 0;

                    auto [ptr, ec] = std::from_chars(sub_view.data(), sub_view.data() + sub_view.size(), idx);
                    if (ec == std::errc() && idx > last_index) {
                        last_index = idx;
                    }
                }
            }
        }

        fileClose(f);

        int next_idx = (last_index == -1) ? 0 : last_index + 1;
        log.info("VFS scan complete for '{}'. Last {} index: {}, next free: {}",
                 file_path, prefix, last_index, next_idx);

        return next_idx;
    }

	void config_patch_apply(fallout::Config* config, const char* file_path) {
		if (config == nullptr || file_path == nullptr) return;

		std::string path_norm = normalize_config_path(file_path);

		auto file_it = ck::g_config_patches.find(path_norm);
		if  (file_it == ck::g_config_patches.end()) return;

		int applied = 0;
		for (const auto& [section, keys] : file_it->second) {
			for (const auto& [key, value] : keys) {
				fallout::configSetString(config, section.c_str(), key.c_str(), value.c_str());
				applied++;
			}
		}

		if (applied > 0) {
			log.info("Applied {} unique patches to: {}", applied, file_path);
		}
	}

	void config_patch_clear() {
		ck::g_config_patches.clear();
		log.info("Cleared all patches.");
	}

}

