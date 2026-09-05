#include "ce_config/ck_config_patch.h"

#include <algorithm>
#include <charconv>

#include "config.h"

#include "ck_log.h"
static const Logger log("CK Config Patch");

namespace fallout {
    bool configSetString(Config*, const char*, const char*, const char*);
    bool wmParseMapsConfig(Config* cfg, int start_map_idx);
    bool wmParseAreasConfig(Config* cfg, int start_area_idx);
    int wmMaxMapIndex();
    int wmMaxAreaIndex();
}

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
    }

    bool apply_worldmap_patches() {
        std::string maps_txt_path = normalize_config_path("data\\maps.txt");
        std::string city_txt_path = normalize_config_path("data\\city.txt");

        // Maps
        fallout::Config maps_cfg;
        if (!fallout::configInit(&maps_cfg)) return false;

        int maps_applied = 0;
        for (const auto& [mod_id, file_maps] : g_config_patches) {
            auto file_it = file_maps.find(maps_txt_path);
            if (file_it != file_maps.end()) {
                for (const auto& [section, keys] : file_it->second) {
                    for (const auto& [key, value] : keys) {
                        fallout::configSetString(&maps_cfg, section.c_str(), key.c_str(), value.c_str());
                        log.info("section: {}, key: {}, value: {}", section, key, value);
                        maps_applied++;
                    }
                }
            }
        }

        if (maps_applied > 0) {
            log.info("Compiling {} map patches into worldmap. Max index before: {}", maps_applied, fallout::wmMaxMapIndex());
            fallout::wmParseMapsConfig(&maps_cfg, fallout::wmMaxMapIndex() + 1);
        }

        fallout::configFree(&maps_cfg);

        // Cities
        fallout::Config city_cfg;
        if (!fallout::configInit(&city_cfg)) return false;

        int city_applied = 0;
        for (const auto& [mod_id, file_maps] : g_config_patches) {
            auto file_it = file_maps.find(city_txt_path);
            if (file_it != file_maps.end()) {
                for (const auto& [section, keys] : file_it->second) {
                    for (const auto& [key, value] : keys) {
                        fallout::configSetString(&city_cfg, section.c_str(), key.c_str(), value.c_str());
                        log.info("section: {}, key: {}, value: {}", section, key, value);
                        city_applied++;
                    }
                }
            }
        }

        if (city_applied > 0) {
            log.info("Compiling {} area patches into worldmap. Max index before: {}", city_applied, fallout::wmMaxAreaIndex());
            fallout::wmParseAreasConfig(&city_cfg, fallout::wmMaxAreaIndex() + 1);
        }

        fallout::configFree(&city_cfg);

        return true;
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

