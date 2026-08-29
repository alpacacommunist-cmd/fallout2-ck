#include "ce_config/ck_config_city.h"
#include "ce_config/ck_config_patch.h"
#include "config.h"

#include <format>
#include <algorithm>
#include <unordered_set>

#include "ck_log.h"
static const Logger log("CK City");

namespace fallout {
    // bool configSetString(Config*, const char*, const char*, const char*);

    bool wmParseMapsConfig(Config* cfg, int start_map_idx);
    bool wmParseAreasConfig(Config* cfg, int start_area_idx);
    int wmMaxMapIndex();
    int wmMaxAreaIndex();
}

namespace ck::config_city {
    static std::unordered_set<std::string> g_registered_area_names;
    static bool g_area_names_cached = false;

    static void cache_original_area_names() {
        if (g_area_names_cached) return;

        fallout::Config cfg;
        if (fallout::configInit(&cfg)) {
            if (fallout::configRead(&cfg, "data\\city.txt", true)) {
                int idx = 0;
                char section[64];
                while (true) {
                    snprintf(section, sizeof(section), "Area %02d", idx);
                    char* area_name = nullptr;
                    if (!fallout::configGetString(&cfg, section, "area_name", &area_name)) break;
                    if (area_name) {
                        std::string area_lower(area_name);
                        std::transform(area_lower.begin(), area_lower.end(), area_lower.begin(), ::tolower);
                        g_registered_area_names.insert(area_lower);
                    }
                    idx++;
                }
            }
            fallout::configFree(&cfg);
        }
        g_area_names_cached = true;
    }

    std::string format_section(int area_id) {
        return std::format("Area {:02d}", area_id);
    }

    int next_index() {
        cache_original_area_names();
        int base_count = fallout::wmMaxAreaIndex() + 1; 

        // 2. Считаем, сколько кастомных секций Area мы уже успели набить в g_config_patches
        std::string city_txt_path = normalize_config_path("data\\city.txt");
        int custom_count = 0;

        for (const auto& [mod_id, file_maps] : g_config_patches) {
            auto file_it = file_maps.find(city_txt_path);
            if (file_it != file_maps.end()) {
                // file_it->second — это map<section, keys>
                // Количество секций в этой мапе — это и есть количество добавленных городов!
                custom_count += file_it->second.size();
            }
        }

        // Твой следующий свободный индекс — это сумма оригинальных + кастомных!
        return base_count + custom_count;
    }

    int next_entrance_index(const char* section_name) {
        std::string city_txt_path = normalize_config_path("data\\city.txt");
        std::string sec_str(section_name);
        int ent_idx = 0;

        // Ищем в нашей мапе, добавлял ли кто-то уже энтрансы в эту секцию
        for (const auto& [mod_id, file_maps] : g_config_patches) {
            auto file_it = file_maps.find(city_txt_path);
            if (file_it != file_maps.end()) {
                auto sec_it = file_it->second.find(sec_str);
                if (sec_it != file_it->second.end()) {
                    // Мы нашли секцию! Теперь считаем ключи, которые начинаются на "entrance_"
                    for (const auto& [key, value] : sec_it->second) {
                        if (key.rfind("entrance_", 0) == 0) { // Проверка, что ключ стартует с "entrance_"
                            ent_idx++;
                        }
                    }
                }
            }
        }

        // Если ключей не было, вернет 0 (подставится entrance_0). 
        // Если один уже был зарегистрирован — вернет 1 (подставится entrance_1).
        return ent_idx;
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

		return target_entrance_id;
    }

	int register_location(const std::string& mod_id, const std::string& name,
                          int world_x, int world_y, const std::string& size) {

        std::string area_lower = name;
        std::transform(area_lower.begin(), area_lower.end(), area_lower.begin(), ::tolower);
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

        return area_index;
    }
}
