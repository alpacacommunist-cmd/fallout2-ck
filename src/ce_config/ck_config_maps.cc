#include "ce_config/ck_config_patch.h"
#include "ce_config/ck_config_maps.h"
#include <format>
#include <string_view>
#include <algorithm>

#include "ck_log.h"
static const Logger log("CK Maps Config");

namespace ck::config_maps {

    std::string format_section(int map_id) {
        return std::format("Map {:03d}", map_id);
    }

    int get_next_index() {
		return ck::config_find_next_free_index("data\\maps.txt", "", "Map");
    }

	int register_map(const std::string& mod_id, const std::string& map_file_name,
				const std::string& name, const std::string& music, const std::string& sfx) {

        std::string map_file_upper = map_file_name;
        std::transform(map_file_upper.begin(), map_file_upper.end(), map_file_upper.begin(), ::toupper);

        std::string maps_txt_path = "data/maps.txt";
        int map_index = -1;

        // check if map is already registered
        for (const auto& [mod, file_maps] : g_config_patches) {
            auto file_it = file_maps.find(normalize_config_path(maps_txt_path));
            if (file_it == file_maps.end()) continue;

            for (const auto& [section, keys] : file_it->second) {
                auto name_it = keys.find("map_name");
                if (name_it != keys.end() && name_it->second == map_file_upper) {
                    map_index = std::stoi(section.substr(4)); // "Map XXX" -> get xxx
                    log.info("Map reload/override detected for '{}': retaining existing ID {}", map_file_name, map_index);
                    break;
                }
            }
            if (map_index != -1) break;
        }

        if (map_index == -1) map_index = get_next_index();

        std::string map_section = format_section(map_index);
        log.info("Mod '{}' registering map: {} (ID: {})", mod_id, map_file_upper, map_index);

        ck::config_patch_add(mod_id, maps_txt_path, map_section, "lookup_name", name);
        ck::config_patch_add(mod_id, maps_txt_path, map_section, "map_name",    map_file_upper);
        ck::config_patch_add(mod_id, maps_txt_path, map_section, "music",       music);
        ck::config_patch_add(mod_id, maps_txt_path, map_section, "ambient_sfx", sfx);
        ck::config_patch_add(mod_id, maps_txt_path, map_section, "saved",       "Yes");

        return map_index;
    }

}
