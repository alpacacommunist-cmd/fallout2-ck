#include "ck_config_patch.h"
#include "db.h"

#include <string>
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
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) { s.remove_suffix(1); }

        if (s.starts_with(search_prefix)) {
            size_t start = search_prefix.size();
            size_t end   = s.find(']', start);

            if (end != std::string_view::npos) {
                std::string_view sub_view = s.substr(start, end - start);
                int idx = 0;
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

namespace ck {
	// : [file][section][key] = value
	ConfigPatchMap g_config_patches;


	int config_next_map_index(const std::string& file_path) {
		int last = ck_find_last_index_vfs(file_path.c_str(), "Map");
		log.info("Last map index: {}", last);
		return last == -1 ? 0 : last + 1;
	}

	int config_next_area_index(const std::string& file_path) {
		int last = ck_find_last_index_vfs(file_path.c_str(), "Area");
		log.info("Last area index: {}", last);
		return last == -1 ? 0 : last + 1;
	}

	int config_find_entrance_by_map_name(std::string_view file_path, std::string_view section, std::string_view map_name) {
        std::string path_norm = normalize_config_path(file_path);
        std::string sec_str(section);

        auto file_it = g_config_patches.find(path_norm);
        if (file_it != g_config_patches.end()) {
            auto sec_it = file_it->second.find(sec_str);
            if (sec_it != file_it->second.end()) {
                for (const auto& [key, val] : sec_it->second) {
                    if (key.rfind("entrance_", 0) == 0 && val.find(map_name) != std::string_view::npos) {
                        // get "5" from "entrance_5"
                        return std::stoi(key.substr(9));
                    }
                }
            }
        }

        return -1;
    }

	int config_count_custom_entrances(std::string_view file_path, std::string_view section) {
        std::string path_norm = normalize_config_path(file_path);
        std::string sec_str(section);
        int count = 0;

        auto file_it = g_config_patches.find(path_norm);
        if (file_it != g_config_patches.end()) {
            auto sec_it = file_it->second.find(sec_str);
            if (sec_it != file_it->second.end()) {
                for (const auto& [key, val] : sec_it->second) {
                    if (key.rfind("entrance_", 0) == 0) count++;
                }
            }
        }
        return count;
    }

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

	int config_count_area_entrances_vfs(int area_id) {
		const char* file_path = "data\\data\\city.txt";

		fallout::File* f = fallout::fileOpen(file_path, "rt");
		if (f == nullptr) {
			log.error("VFS cannot open: {}", file_path);
			return 0;
		}

		std::string target_section = "[Area ";
		if (area_id < 10)  target_section += "0";
		target_section += std::to_string(area_id) + "]";

		char line[1024]; bool in_target_section = false; int entrance_count = 0;

		while (fileReadString(line, sizeof(line), f) != nullptr) {
			std::string_view s(line);

			size_t comment_pos = s.find(';');
			if (comment_pos != std::string_view::npos) s = s.substr(0, comment_pos);

			while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t'))
				s.remove_suffix(1);

			while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
				s.remove_prefix(1);

			if (s.empty()) continue;

			// was in target section, found a new one - break
			if (s.starts_with("[") && in_target_section) break;

			if (s == target_section) {
				in_target_section = true;
				continue;
			}

			if (in_target_section) {
				if (s.starts_with("entrance_")) entrance_count++;
			}
		}

		fileClose(f);
		log.info("Found {} original entrances in VFS for area {}", entrance_count, area_id);
		return entrance_count;
	}


}

void ck_config_patch_add(std::string_view file_path, std::string_view section,
                         std::string_view key, std::string_view value) {
    std::string path_norm = normalize_config_path(file_path);
    std::string sec_str(section);
    std::string key_str(key);
    std::string val_str(value);

    // check if existing patch is being overridden
    if (ck::g_config_patches[path_norm][sec_str].contains(key_str)) {
        log.warn("Config key override! [{}] {} -> changed from '{}' to '{}'",
                 section, key, ck::g_config_patches[path_norm][sec_str][key_str], value);
    } else {
        log.info("Registered patch: [{}] {} = {} ({})", section, key, value, path_norm);
    }

	ck::g_config_patches[path_norm][sec_str][key_str] = val_str;
}

void ck_config_patch_apply(fallout::Config* config, const char* file_path) {
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

void ck_config_patch_clear() {
	ck::g_config_patches.clear();
    log.info("Cleared all patches.");
}
