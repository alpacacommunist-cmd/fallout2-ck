#include "ce_config/ck_config_city.h"
#include "ce_config/ck_config_patch.h"
#include "ck_messages/ck_messages.h"

#include "db.h"

#include <format>
#include <algorithm>

#include "ck_log.h"
static const Logger log("CK City");

namespace ck::config_city {

    std::string format_section(int area_id) {
        return std::format("Area {:02d}", area_id);
    }

	int get_next_index() {
        return ck::config_find_next_free_index("data\\data\\city.txt", "", "Area");
    }

    int count_original_entrances(int area_id) {
        const char* file_path = "data\\data\\city.txt";
        fallout::File* f = fallout::fileOpen(file_path, "rt");
        if (f == nullptr) {
            log.error("VFS cannot open city.txt");
            return 0;
        }

        std::string target_section = "[" + format_section(area_id) + "]";
        char line[1024];
        bool in_target_section = false;
        int entrance_count = 0;

        while (fileReadString(line, sizeof(line), f) != nullptr) {
            std::string_view s(line);

            size_t comment_pos = s.find(';');
            if (comment_pos != std::string_view::npos) s = s.substr(0, comment_pos);

            while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
            while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);

            if (s.empty()) continue;
            if (s.starts_with("[") && in_target_section) break;

            if (s == target_section) {
                in_target_section = true;
                continue;
            }

            if (in_target_section && s.starts_with("entrance_")) {
                entrance_count++;
            }
        }

        fileClose(f);
        log.info("Found {} original entrances in VFS for area {}", entrance_count, area_id);
        return entrance_count;
    }

    int expand_location(const std::string& mod_id, int area_id,
                        const std::string& map_lookup_name, int x, int y) {
		std::string area_section = format_section(area_id);
		std::string city_path = "data/city.txt";
		// std::string city_path = "data\\data\\city.txt";
		// std::string city_path = "data\\city.txt";

		int target_entrance_id = ck::config_find_next_free_index(city_path, area_section, "entrance_");

		std::string entrance_key = "entrance_" + std::to_string(target_entrance_id);
		std::string entrance_value = std::format("On,{},{},{},-1,-1,0", x, y, map_lookup_name);

		log.info("Mod '{}' registering: [{}] {} = {}", mod_id, area_section, entrance_key, entrance_value);

		ck::config_patch_add(mod_id, city_path, area_section, entrance_key, entrance_value);

		return target_entrance_id;
    }

	int register_location(const std::string& mod_id, const std::string& name,
                          int world_x, int world_y, const std::string& size) {

        int next_area_index = -1;
        if (next_area_index == -1) next_area_index = get_next_index();

        std::string city_path = "data/city.txt";
        int area_idx = -1;

        if (area_idx == -1) area_idx = next_area_index++;

        std::string area_section = format_section(area_idx);
        std::string world_pos = std::format("{},{}", world_x, world_y);

        log.info("Registered worldmap area: {} (ID: {})", name, area_idx);

        ck::config_patch_add(mod_id, city_path, area_section, "area_name",             name);
        ck::config_patch_add(mod_id, city_path, area_section, "world_pos",             world_pos);
        ck::config_patch_add(mod_id, city_path, area_section, "start_state",           "On");
        ck::config_patch_add(mod_id, city_path, area_section, "size",                  size);
        ck::config_patch_add(mod_id, city_path, area_section, "townmap_art_idx",       "-1");
        ck::config_patch_add(mod_id, city_path, area_section, "townmap_label_art_idx", "-1");

        return area_idx;
    }
}
