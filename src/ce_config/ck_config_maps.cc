#include "ce_config/ck_config_patch.h"
#include "ce_config/ck_config_maps.h"
#include "config.h"
#include <format>
#include <algorithm>
#include <unordered_set>

#include "ck_log.h"
static const Logger log("CK Maps Config");

namespace ck::config_maps {
    static std::unordered_set<std::string> g_registered_lookup_names;
    static int current_maps_count = 0;

    std::string format_section(int map_id) {
        return std::format("Map {:03d}", map_id);
    }

    int next_index() {
        return current_maps_count;
    }

    // Reads maps.txt (before fallout2-ce does), stores count in `current_maps_num`
    // and `lookup_name` in `g_registered_lookup_names`
    bool preprocess_maps() {
        fallout::Config cfg;

        if (!fallout::configInit(&cfg)) return false;
        if (!fallout::configRead(&cfg, "data\\maps.txt", true)) return false;

        int idx = 0;
        char section[64];

        while (true) {
            snprintf(section, sizeof(section), "Map %03d", idx);
            char* lookup_name = nullptr;

            if (!fallout::configGetString(&cfg, section, "lookup_name", &lookup_name)) break;

            if (lookup_name) {
                std::string name_lower(lookup_name);
                std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                g_registered_lookup_names.insert(name_lower);
            }

            idx++;
            current_maps_count++;
        }

        fallout::configFree(&cfg);

        return true;
    }

	int register_map(const std::string& mod_id, const std::string& map_file_name,
				const std::string& name, const std::string& music, const std::string& sfx) {

        std::string name_lower = name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
        int map_index = next_index();

        if (g_registered_lookup_names.find(name_lower) != g_registered_lookup_names.end()) {
            log.error("Mod '{}' failed to register map! lookup_name '{}' is already in use!",
                    mod_id, name);

            return -1;
        }

        std::string map_file_upper = map_file_name;
        std::transform(map_file_upper.begin(), map_file_upper.end(), map_file_upper.begin(), ::toupper);

        std::string maps_txt_path = "data/maps.txt";

        std::string map_section = format_section(map_index);
        log.info("Mod '{}' registering map: {} (ID: {})", mod_id, map_file_upper, map_index);

        g_registered_lookup_names.insert(name_lower);

        ck::config_patch_add(mod_id, maps_txt_path, map_section, "lookup_name", name);
        ck::config_patch_add(mod_id, maps_txt_path, map_section, "map_name",    map_file_upper);
        ck::config_patch_add(mod_id, maps_txt_path, map_section, "music",       music);
        ck::config_patch_add(mod_id, maps_txt_path, map_section, "ambient_sfx", sfx);
        ck::config_patch_add(mod_id, maps_txt_path, map_section, "saved",       "Yes");

        current_maps_count++;
        return map_index;
    }
}
