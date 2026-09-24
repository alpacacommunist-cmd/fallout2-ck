#include "ce_config/ck_config_city.h"
#include "ce_config/ck_config_patch.h"
#include "config.h"

#include <format>
#include <algorithm>
#include <unordered_set>

#include "ck_log.h"
static const Logger log("CK City");

namespace ck::config_city {
    static std::unordered_set<std::string> g_registered_area_names;
    static std::unordered_map<std::string, int> g_area_entrances_cache;
    static int current_areas_num = 0;

    std::string to_lower(std::string string) {
        std::transform(string.begin(), string.end(), string.begin(), ::tolower);
        return string;
    }

    std::string format_section(int area_id) {
        return std::format("Area {:02d}", area_id);
    }

    int next_index() {
        return current_areas_num;
    }

    // Reads city.txt (before fallout2-ce does), stores count in `current_areas_num`
    // and `area_name` in `g_registered_area_names`
    bool preprocess_areas() {
        if (current_areas_num > 0) return false;

        fallout::Config cfg;

        if (!fallout::configInit(&cfg)) return false;
        if (!fallout::configRead(&cfg, "data\\city.txt", true)) return false;

        int idx = 0;
        char section[64];

        while (true) {
            snprintf(section, sizeof(section), "Area %02d", idx);
            char* area_name = nullptr;

            if (!fallout::configGetString(&cfg, section, "area_name", &area_name)) break;
            if (area_name) {
                g_registered_area_names.insert(to_lower(area_name));

                // entrances count
                int entrances = 0;
                char key[64];
                while (true) {
                    snprintf(key, sizeof(key), "entrance_%d", entrances);
                    char* dummy_str = nullptr;
                    if (!fallout::configGetString(&cfg, section, key, &dummy_str)) break;
                    entrances++;
                }

                g_area_entrances_cache[to_lower(section)] = entrances;
            }

            idx++;
            current_areas_num++;
        }

        fallout::configFree(&cfg);
        return true;
    }

    int next_entrance_index(const char* section_name) {
        std::string sec_lower = to_lower(section_name);

        auto it = g_area_entrances_cache.find(sec_lower);
        if (it != g_area_entrances_cache.end()) {
            return it->second;
        }

        return 0;
    }

    int expand_location(const std::string& mod_id, int area_id,
                        const std::string& map_lookup_name, int x, int y) {
		std::string area_section = format_section(area_id);
		std::string city_path = "data/city.txt";

		int target_entrance_id = next_entrance_index(area_section.c_str());

		std::string entrance_key = "entrance_" + std::to_string(target_entrance_id);
		std::string entrance_value = std::format("On,{},{},{},-1,-1,0", x, y, map_lookup_name);

		log.info("Mod '{}' registering: [{}] {} = {}", mod_id, area_section, entrance_key, entrance_value);

		ck::config_patch_add(mod_id, city_path, area_section, entrance_key, entrance_value);

        g_area_entrances_cache[to_lower(area_section)] = target_entrance_id + 1;
		return target_entrance_id;
    }

	int register_location(const std::string& mod_id, const std::string& name,
                          int world_x, int world_y, const std::string& size) {

        std::string area_lower = to_lower(name);
        int area_index = next_index();

        if (g_registered_area_names.find(area_lower) != g_registered_area_names.end()) {
            log.error("Mod '{}' failed to register location! area_name '{}' is already in use!",
                    mod_id, name);
            return -1;
        }

        std::string city_path = "data/city.txt";

        std::string area_section = format_section(area_index);
        std::string world_pos = std::format("{},{}", world_x, world_y);

        log.info("Registered worldmap area: {} (ID: {})", name, area_index);

        g_registered_area_names.insert(area_lower);

        ck::config_patch_add(mod_id, city_path, area_section, "area_name",             name);
        ck::config_patch_add(mod_id, city_path, area_section, "world_pos",             world_pos);
        ck::config_patch_add(mod_id, city_path, area_section, "start_state",           "On");
        ck::config_patch_add(mod_id, city_path, area_section, "size",                  size);
        ck::config_patch_add(mod_id, city_path, area_section, "townmap_art_idx",       "-1");
        ck::config_patch_add(mod_id, city_path, area_section, "townmap_label_art_idx", "-1");

        current_areas_num++;
        return area_index;
    }
}
