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
        return ck::config_find_next_free_index_vfs("data\\data\\city.txt", "Area");
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
        int target_entrance_id = -1;

        // search existing custom entrance for this map
        auto file_it = ck::g_config_patches.find(city_path);
        if (file_it != ck::g_config_patches.end()) {
            auto sec_it = file_it->second.find(area_section);
            if (sec_it != file_it->second.end()) {
                for (const auto& [key, val] : sec_it->second) {
                    if (key.rfind("entrance_", 0) == 0 && val.find(map_lookup_name) != std::string::npos) {
                        target_entrance_id = std::stoi(key.substr(9));
                        break;
                    }
                }
            }
        }

        if (target_entrance_id == -1) {
            int original_entrances = count_original_entrances(area_id);
            int custom_entrances = 0;

            if (file_it != ck::g_config_patches.end()) {
                auto sec_it = file_it->second.find(area_section);
                if (sec_it != file_it->second.end()) {
                    for (const auto& [key, val] : sec_it->second) {
                        if (key.rfind("entrance_", 0) == 0) custom_entrances++;
                    }
                }
            }

            target_entrance_id = original_entrances + custom_entrances;
            log.info("Mod reload detected for [{}]: updating existing entrance_{}", area_section, target_entrance_id);
        }

        std::string entrance_key = "entrance_" + std::to_string(target_entrance_id);
        std::string entrance_value = std::format("On,{},{},{},-1,-1,0", x, y, map_lookup_name);

        ck::config_patch_add(city_path, area_section, entrance_key, entrance_value);

        int townmap_msg_id = 200 + (area_id * 10) + target_entrance_id;
        ck::messages_add_string("worldmap.msg", townmap_msg_id, map_lookup_name);

        return target_entrance_id;
    }

	int register_location(const std::string& mod_id, const std::string& name,
                          int world_x, int world_y, const std::string& size) {

        static int next_area_index = -1;
        if (next_area_index == -1) {
            next_area_index = get_next_index();
        }

        std::string city_path = "data/city.txt";
        int area_idx = -1;

        // Mod Reload check
        auto file_it = g_config_patches.find(city_path);
        if (file_it != g_config_patches.end()) {
            for (const auto& [section, keys] : file_it->second) {
                auto name_it = keys.find("area_name");
                if (name_it != keys.end() && name_it->second == name) {
                    area_idx = std::stoi(section.substr(5));
                    log.info("Area reload detected for '{}': retaining existing ID {}", name, area_idx);
                    break;
                }
            }
        }

        if (area_idx == -1) area_idx = next_area_index++;

        std::string area_section = format_section(area_idx);
        std::string world_pos = std::format("{},{}", world_x, world_y);

        log.info("Registered worldmap area: {} (ID: {})", name, area_idx);

        ck::config_patch_add(city_path, area_section, "area_name",             name);
        ck::config_patch_add(city_path, area_section, "world_pos",             world_pos);
        ck::config_patch_add(city_path, area_section, "start_state",           "On");
        ck::config_patch_add(city_path, area_section, "size",                  size);
        ck::config_patch_add(city_path, area_section, "townmap_art_idx",       "-1");
        ck::config_patch_add(city_path, area_section, "townmap_label_art_idx", "-1");

        ck::messages_add_string("map.msg", 1500 + area_idx, name);

        return area_idx;
    }
}
