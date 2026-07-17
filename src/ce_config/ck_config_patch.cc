#include "ce_config/ck_config_patch.h"
#include "db.h"

#include <algorithm>
#include <charconv>

#include "ck_log.h"
static const Logger log("CK Config Patch");

namespace fallout {
    bool configSetString(Config*, const char*, const char*, const char*);
}

namespace ck {
	// : [mod_id][file][section][key] = value
	ConfigPatchMap g_config_patches;

	std::string normalize_config_path(std::string_view path) {
		std::string result(path);
		for (char& c : result) if (c == '\\') c = '/';

		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}

	void config_patch_add(std::string_view mod_id, std::string_view file_path,
				std::string_view section, std::string_view key, std::string_view value) {

        std::string path_norm = normalize_config_path(file_path);

        g_config_patches[std::string(mod_id)][path_norm][std::string(section)][std::string(key)] = std::string(value);
        log.info("Registered patch for mod '{}': [{}] {} = {}", mod_id, section, key, value);
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

	int config_find_next_free_key_index(std::string_view file_path, std::string_view section, std::string_view key_prefix) {
		std::string path_norm = normalize_config_path(file_path);
		std::string sec_str(section);
		std::string prefix_str(key_prefix); // eg "entrance_"

		int max_index = -1;

		fallout::File* f = fallout::fileOpen(path_norm.c_str(), "rt");
		if (f != nullptr) {
			char line[1024];
			bool in_target_section = false;

			while (fileReadString(line, sizeof(line), f) != nullptr) {
				std::string_view s(line);

				size_t comment_pos = s.find(';');
				if (comment_pos != std::string_view::npos) {
					s = s.substr(0, comment_pos);
				}

				while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
				while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
				if (s.empty()) continue;

				if (s.starts_with("[") && s.ends_with("]")) {
					std::string_view current_sec = s.substr(1, s.size() - 2);

					while (!current_sec.empty() && (current_sec.back() == ' ' || current_sec.back() == '\t')) current_sec.remove_suffix(1);
					while (!current_sec.empty() && (current_sec.front() == ' ' || current_sec.front() == '\t')) current_sec.remove_prefix(1);

					in_target_section = (current_sec == sec_str);
					continue;
				}

				if (in_target_section && s.starts_with(prefix_str)) {
					size_t end_of_num = s.find_first_of(" =\t", prefix_str.size());

					if (end_of_num != std::string_view::npos && end_of_num > prefix_str.size()) {
						std::string_view idx_view = s.substr(prefix_str.size(), end_of_num - prefix_str.size());

						int idx = 0;
						auto [ptr, ec] = std::from_chars(idx_view.data(), idx_view.data() + idx_view.size(), idx);
						if (ec == std::errc() && idx > max_index) {
							max_index = idx;
						}
					}
				}
			}
			fileClose(f);
		}

		for (const auto& [mod, file_maps] : g_config_patches) {
			auto file_it = file_maps.find(path_norm);
			if (file_it == file_maps.end()) continue;

			auto sec_it = file_it->second.find(sec_str);
			if (sec_it == file_it->second.end()) continue;

			for (const auto& [key, val] : sec_it->second) {
				if (key.starts_with(prefix_str)) {
					std::string_view idx_view(key.data() + prefix_str.size(), key.size() - prefix_str.size());

					int idx = 0;
					auto [ptr, ec] = std::from_chars(idx_view.data(), idx_view.data() + idx_view.size(), idx);
					if (ec == std::errc() && idx > max_index) {
						max_index = idx;
					}
				}
			}
		}

		return (max_index == -1) ? 0 : max_index + 1;
	}

	void config_patch_apply(fallout::Config* config, const char* file_path) {
		if (config == nullptr || file_path == nullptr) return;

		std::string path_norm = normalize_config_path(file_path);
		int applied = 0;

		for (const auto& [mod_id, file_maps] : g_config_patches) {

			auto file_it = file_maps.find(path_norm);

			if (file_it == file_maps.end()) {
				if (path_norm == "data/city.txt") file_it = file_maps.find("data/data/city.txt");
				else if (path_norm == "data/data/city.txt") file_it = file_maps.find("data/city.txt");
				else if (path_norm == "data/maps.txt") file_it = file_maps.find("data/data/maps.txt");
				else if (path_norm == "data/data/maps.txt") file_it = file_maps.find("data/maps.txt");
			}

			if (file_it != file_maps.end()) {
				for (const auto& [section, keys] : file_it->second) {
					for (const auto& [key, value] : keys) {
						fallout::configSetString(config, section.c_str(), key.c_str(), value.c_str());
						log.info("applying: section -> {}, key -> {}, value -> {}", section, key, value);
						applied++;
					}
				}
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

	void clear_config_patches_for_mod(const char* mod_id) {
        if (mod_id == nullptr) return;
        std::string mod_str(mod_id);

        auto it = g_config_patches.find(mod_str);
        if (it != g_config_patches.end()) {
            g_config_patches.erase(it);
            log.info("Successfully cleared all configuration patches for mod: {}", mod_id);
        }
    }

}

void ck_config_clear_mod_patches(const char* mod_id) {
	ck::clear_config_patches_for_mod(mod_id);
}

