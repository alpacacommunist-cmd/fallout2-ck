#include "map.h"

#include "ce_config/ck_config_patch.h"
#include "ck_messages/ck_messages.h"
#include "ck_area/ck_area.h"

#include <unordered_map>
#include <algorithm>
#include <format>
#include <cstring>

#include "ck_log.h"
static const Logger log("CK Locations");

namespace ck {
	static int g_current_loading_map_id = -1;

	static std::unordered_map<int, int> g_map_id_to_original;
	static std::unordered_map<int, int> gMapIdRedirects;           // orig_ID -> ck_ID
    static std::unordered_map<std::string, std::string> gMapPaths; // lowercase_name -> full path
    static std::unordered_map<std::string, int> gRuntimeMaps;      // lowercase_name -> ck_id_ID

	int area_resolve_map_id(int original_map_id) {
		auto it = gMapIdRedirects.find(original_map_id);
		if (it != gMapIdRedirects.end()) {
			g_current_loading_map_id = it->second;
			return it->second;
		}

		g_current_loading_map_id = -1;
		return original_map_id;
	}

	int area_resolve_id_for_city_match(int map_index) {
        auto it = g_map_id_to_original.find(map_index);
        if (it != g_map_id_to_original.end()) {
            log.debug("Translating dynamic map ID {} back to original {} for worldmap area match", map_index, it->second);
            return it->second;
        }
        return map_index;
    }

	const char* area_resolve_path(const char* name) {
		if (name == nullptr) return nullptr;
		static char resolved_path[512];

		std::string full_name = name;
		size_t dot = full_name.find('.');

		// name ("tstcv") ext (".EDG", ".MAP")
		std::string key = (dot != std::string::npos) ? full_name.substr(0, dot) : full_name;
		std::string ext = (dot != std::string::npos) ? full_name.substr(dot) : "";

		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

		if (ext.empty()) ext = ".MAP";
		if (ext != ".MAP" && ext != ".EDG" && ext != ".MSK") {
			return nullptr;
		}

		auto it = gMapPaths.find(key);
		if (it == gMapPaths.end()) return nullptr; // map's not ours, skip to engine

		// loaded through something other than mapLoadById
		// restore current map id
		if (g_current_loading_map_id == -1) {
			g_current_loading_map_id = gRuntimeMaps[key];
		}

		// gMapPaths[key] keeps path "../mods/mod_id/maps/TSTCV.MAP"
		// change .map ext to an ext engine wants (edg/msk)
		std::string base_path = it->second;
		size_t path_dot = base_path.find_last_of('.');
		if (path_dot != std::string::npos) base_path = base_path.substr(0, path_dot);

		// final path
		std::string final_path = base_path + ext;

		strncpy(resolved_path, final_path.c_str(), sizeof(resolved_path) - 1);
		resolved_path[sizeof(resolved_path) - 1] = '\0';

		log.debug("Redirecting asset path for map: {} -> {}", name, resolved_path);
		return resolved_path;
	}

	void area_on_map_header_set(fallout::MapHeader* header) {
		if (g_current_loading_map_id != -1 && header != nullptr) {
			log.info("Memory patching map header index: {} -> {}", header->index, g_current_loading_map_id);

			header->index = g_current_loading_map_id;
		}

		g_current_loading_map_id = -1;
	}

	int area_register_map(const std::string& map_file_name, const std::string& name,
			   const std::string& sub_name, const std::string& music) {

        static int next_map_index = -1;
        if (next_map_index == -1) {
            next_map_index = ck::config_next_map_index("data\\data\\maps.txt");
        }

        std::string map_file_upper = map_file_name;
        std::transform(map_file_upper.begin(), map_file_upper.end(), map_file_upper.begin(), ::toupper);
        std::string map_file_lower = map_file_name;
        std::transform(map_file_lower.begin(), map_file_lower.end(), map_file_lower.begin(), ::tolower);

        int map_index = -1;

        // (Mod Reload Check):
        auto runtime_it = gRuntimeMaps.find(map_file_lower);
        if (runtime_it != gRuntimeMaps.end()) {
            map_index = runtime_it->second; // get previous id (eg 151)
            log.info("Map reload detected for [{}]: retaining existing ID {}", map_file_upper, map_index);
        } else map_index = next_map_index++;

        std::string map_file_path = std::format("../mods/{}/maps/{}.MAP", ck_get_current_mod_id(), map_file_upper);
        gMapPaths[map_file_lower] = map_file_path;
        gRuntimeMaps[map_file_lower] = map_index;

        log.info("Registered map: {} (ID: {}) -> {}", map_file_upper, map_index, map_file_path);

        std::string mapSection = "Map " + std::to_string(map_index);
        ck::config_patch_add("data\\maps.txt", mapSection, "lookup_name", name);
        ck::config_patch_add("data\\maps.txt", mapSection, "map_name",    map_file_upper);
        ck::config_patch_add("data\\maps.txt", mapSection, "music",       music);
		ck::config_patch_add("data\\maps.txt", mapSection, "saved",       "Yes");

        int map_base_id = map_index * 3;
        ck::messages_add_string("map.msg", map_base_id + 100, name);     // name
        ck::messages_add_string("map.msg", map_base_id + 101, name);     // save label

        std::string full_description = sub_name.empty() ? name : sub_name;
        ck::messages_add_string("map.msg", map_base_id + 200, full_description); // description

        return map_index;
    }

	int area_override_map(int original_map_id, const CkAreaMapFFI& data) {
        std::string map_file  = data.map_file ? data.map_file  : std::string();
        std::string name      = data.name     ? data.name      : std::string();
        std::string sub_name  = data.sub_name ? data.sub_name  : std::string();
        std::string music     = data.music    ? data.music     : "17arroyo";

        int map_id = ck::area_register_map(map_file, name, sub_name, music);
        gMapIdRedirects[original_map_id] = map_id;
		g_map_id_to_original[map_id] = original_map_id;

        log.info("Redirect established: original ID {} -> {}", original_map_id, map_id);
        return map_id;
    }

	int expand_location(int area_id, const std::string& custom_map_lookup_name, int townmap_x, int townmap_y) {
        // format section [Area 00]
        std::string area_section = std::format("Area {:02d}", area_id);
        std::string city_path = "data/city.txt";

        int next_entrance_id = ck::config_find_entrance_by_map_name(city_path, area_section, custom_map_lookup_name);

        if (next_entrance_id == -1) {
            // first map registration, get free index
            int original_entrances = ck::config_count_area_entrances_vfs(area_id);
            int custom_entrances   = ck::config_count_custom_entrances(city_path, area_section);

            next_entrance_id = original_entrances + custom_entrances;
        } else {
            log.info("Mod reload detected for [{}]: updating existing entrance_{}", area_section, next_entrance_id);
        }

        std::string entrance_key   = "entrance_" + std::to_string(next_entrance_id);
        std::string entrance_value = std::format("On,{},{},{},-1,-1,0", townmap_x, townmap_y, custom_map_lookup_name);

        ck::config_patch_add(city_path, area_section, entrance_key, entrance_value);
        log.info("Successfully patched [{}] with {} = {}", area_section, entrance_key, entrance_value);

        int townmap_msg_id = 200 + (10 * area_id) + next_entrance_id;
		ck::messages_add_string("worldmap.msg", townmap_msg_id, custom_map_lookup_name);

        return next_entrance_id;
    }

	int area_register_location(const std::string& name,
			int worldX, int worldY, const std::string& size,
			const std::vector<std::string>& entranceLookups) {

		static int nextAreaIdx = -1;
		if (nextAreaIdx == -1) {
			nextAreaIdx = ck::config_next_area_index("data\\data\\city.txt");
		}

		int areaIdx = nextAreaIdx++;

		std::string areaSection = "Area " + std::to_string(areaIdx);
		std::string worldPos = std::format("{},{}", worldX, worldY);

		log.info("Registered worldmap area: {} (ID: {})", name, areaIdx);

		ck::config_patch_add("data\\city.txt", areaSection, "area_name",             name);
		ck::config_patch_add("data\\city.txt", areaSection, "world_pos",             worldPos);
		ck::config_patch_add("data\\city.txt", areaSection, "start_state",           "On");
		ck::config_patch_add("data\\city.txt", areaSection, "size",                  size);
		ck::config_patch_add("data\\city.txt", areaSection, "townmap_art_idx",       "-1");
		ck::config_patch_add("data\\city.txt", areaSection, "townmap_label_art_idx", "-1");

		// entrances
		for (size_t i = 0; i < entranceLookups.size(); ++i) {
			std::string entrance = std::format("On,350,275,{},-1,-1,3", entranceLookups[i]);
			ck::config_patch_add("data\\city.txt", areaSection, "entrance_" + std::to_string(i), entrance);
		}

		// city name
		ck::messages_add_string("game/map.msg", 1500 + areaIdx, name);

		return areaIdx;
	}

}

int ck_area_override_map(int original_map_id, const CkAreaMapFFI* data) {
	if (!data) return -1;

    return ck::area_override_map(original_map_id, *data);
}

int ck_area_expand_location(int area_id, const char* custom_map_lookup_name, int townmap_x, int townmap_y) {
	if (!custom_map_lookup_name) return -1;

	return ck::expand_location(area_id, custom_map_lookup_name, townmap_x, townmap_y);
}

int ck_area_register_map(const CkAreaMapFFI* data) {
    if (!data) return -1;

    return ck::area_register_map(
        data->map_file ? data->map_file : std::string(),
        data->name     ? data->name     : std::string(),
        data->sub_name ? data->sub_name : std::string(),
        data->music    ? data->music    : std::string("17arroyo")
    );
}

int ck_area_register_area(const char* name, int worldX, int worldY,
		const char* size, const char** entranceLookups, int entranceCount) {

	std::vector<std::string> lookups;
	for (int i = 0; i < entranceCount; ++i) lookups.push_back(entranceLookups[i]);

	return ck::area_register_location(name, worldX, worldY, size, lookups);
}
