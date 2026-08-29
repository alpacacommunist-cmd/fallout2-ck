#include "ce_config/ck_config_patch.h"
#include "ce_config/ck_config_maps.h"
#include "config.h"
#include <format>
#include <algorithm>
#include <unordered_set>

#include "ck_log.h"
static const Logger log("CK Maps Config");

namespace fallout {
    // bool configSetString(Config*, const char*, const char*, const char*);

    bool wmParseMapsConfig(Config* cfg, int start_map_idx);
    bool wmParseAreasConfig(Config* cfg, int start_area_idx);
    int wmMaxMapIndex();
    int wmMaxAreaIndex();
}

namespace ck::config_maps {
    static std::unordered_set<std::string> g_registered_lookup_names;
    static bool g_map_names_cached = false;

    void cache_original_map_names() {
        if (g_map_names_cached) return;

        fallout::Config cfg;
        if (fallout::configInit(&cfg)) {
            if (fallout::configRead(&cfg, "data\\maps.txt", true)) {
                int idx = 0;
                char section[64];
                while (true) {
                    snprintf(section, sizeof(section), "Map %03d", idx);
                    char* lookup_name = nullptr;

                    // Ищем по ключу lookup_name
                    if (!fallout::configGetString(&cfg, section, "lookup_name", &lookup_name)) break;
                    if (lookup_name) {
                        std::string map_lower(lookup_name);
                        std::transform(map_lower.begin(), map_lower.end(), map_lower.begin(), ::tolower);
                        g_registered_lookup_names.insert(map_lower);
                    }
                    idx++;
                }
            }
            fallout::configFree(&cfg);
        }
        g_map_names_cached = true;
    }

    std::string format_section(int map_id) {
        return std::format("Map {:03d}", map_id);
    }

    int next_index() {
        cache_original_map_names();
        // 1. База — это сколько карт загрузил оригинальный движок из maps.txt
        int base_count = fallout::wmMaxMapIndex() + 1; 

        // 2. Считаем, сколько секций Map мы уже успели набить в g_config_patches
        std::string maps_txt_path = normalize_config_path("data\\maps.txt");
        int custom_count = 0;

        for (const auto& [mod_id, file_maps] : g_config_patches) {
            auto file_it = file_maps.find(maps_txt_path);
            if (file_it != file_maps.end()) {
                // file_it->second — это map<section, keys>
                custom_count += file_it->second.size();
            }
        }

        // Твой следующий свободный индекс карт
        return base_count + custom_count;
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

        return map_index;
    }

}
