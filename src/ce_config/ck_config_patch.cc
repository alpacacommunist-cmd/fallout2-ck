#include "ce_config/ck_config_patch.h"
#include "db.h"

#include <algorithm>
#include <charconv>

#include "ck_log.h"
static const Logger log("CK Config Patch");

namespace {
    inline std::string_view trim_string(std::string_view s) {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);

        return s;
    }

    inline std::string_view strip_comments(std::string_view s) {
        size_t comment_pos = s.find(';');
        if (comment_pos != std::string_view::npos) s = s.substr(0, comment_pos);

        return s;
    }

    inline bool try_parse_int(std::string_view s, int& out_value) {
        int val = 0;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
        if (ec == std::errc()) {
            out_value = val;
            return true;
        }
        return false;
    }
}

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

		while (result.rfind("data/", 0) == 0) result = result.substr(5);

		// "data\\data\\city.txt" -> "city.txt"
		// "data/maps.txt"        -> "maps.txt"
		return result;
	}

	void config_patch_add(std::string_view mod_id, std::string_view file_path,
				std::string_view section, std::string_view key, std::string_view value) {

        std::string path_norm = normalize_config_path(file_path);

        g_config_patches[std::string(mod_id)][path_norm][std::string(section)][std::string(key)] = std::string(value);
        log.info("Registered patch for mod '{}': [{}] {} = {}", mod_id, section, key, value);
    }

	int config_find_next_free_index(std::string_view file_path, std::string_view section, std::string_view prefix) {
		std::string path_norm = normalize_config_path(file_path);
		std::string sec_str(section);
		std::string prefix_str(prefix);

		int max_index = -1;
		const bool scanning_sections = sec_str.empty();

		fallout::File* f = fallout::fileOpen(path_norm.c_str(), "rt");
		if (f == nullptr) f = fallout::fileOpen(("data\\data\\" + path_norm).c_str(), "rt");

		if (f != nullptr) {
			char line[1024];
			bool in_target_section = scanning_sections;

			while (fileReadString(line, sizeof(line), f) != nullptr) {
				std::string_view s = trim_string(strip_comments(line));
				if (s.empty()) continue;

				// section header [...]
				if (s.starts_with("[") && s.ends_with("]")) {
					std::string_view current_sec = trim_string(s.substr(1, s.size() - 2));

					if (scanning_sections) {
						if (current_sec.starts_with(prefix_str)) {
							// ФИКС: Обязательно триммим остаток строки, чтобы убрать пробел перед числом!
							std::string_view idx_view = trim_string(current_sec.substr(prefix_str.size()));
							int idx = 0;
							if (try_parse_int(idx_view, idx)) {
								if (idx > max_index) max_index = idx;
							}
						}
					} else {
						in_target_section = (current_sec == sec_str);
					}
					continue;
				}

				// keys eg entrance_4=...
				if (!scanning_sections && in_target_section && s.starts_with(prefix_str)) {
					size_t end_of_num = s.find_first_of(" =\t", prefix_str.size());
					if (end_of_num != std::string_view::npos && end_of_num > prefix_str.size()) {
						int idx = 0;
						if (try_parse_int(s.substr(prefix_str.size(), end_of_num - prefix_str.size()), idx)) {
							if (idx > max_index) max_index = idx;
						}
					}
				}
			}
			fileClose(f);
		}

		// check g_config_patches
		for (const auto& [mod, file_maps] : g_config_patches) {
			auto file_it = file_maps.find(path_norm);
			if (file_it == file_maps.end()) continue;

			if (scanning_sections) {
				// looking for new section, check existing mod patches for section name
				for (const auto& [existing_section, keys] : file_it->second) {
					if (existing_section.starts_with(prefix_str)) {
						std::string_view idx_view = trim_string(existing_section.substr(prefix_str.size()));
						int idx = 0;
						if (try_parse_int(idx_view, idx)) {
							if (idx > max_index) max_index = idx;
						}
					}
				}
			} else {
				// looking for free key (entrance_X) in particular section
				auto sec_it = file_it->second.find(sec_str);
				if (sec_it == file_it->second.end()) continue;

				for (const auto& [key, val] : sec_it->second) {
					if (key.starts_with(prefix_str)) {
						int idx = 0;
						if (try_parse_int(std::string_view(key.data() + prefix_str.size(), key.size() - prefix_str.size()), idx)) {
							if (idx > max_index) max_index = idx;
						}
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

