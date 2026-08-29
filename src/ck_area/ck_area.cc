#include "ck_area/ck_area.h"

#include "ce_config/ck_config_maps.h"
#include "ce_config/ck_config_city.h"
#include "ck_messages/ck_messages.h"

#include <unordered_map>
#include <algorithm>
#include <format>
#include <cstring>

#include "map.h"

#include "ck_log.h"
static const Logger logger("CK Locations");

namespace ck {
	static int g_current_loading_map_id = -1;

	static std::unordered_map<int, int> g_map_id_to_original;
	static std::unordered_map<int, int> g_map_id_redirects;           // orig_ID -> ck_ID
    static std::unordered_map<std::string, std::string> gMapPaths; // lowercase_name -> full path
    static std::unordered_map<std::string, int> gRuntimeMaps;      // lowercase_name -> ck_id_ID

	// int area_resolve_map_id(int original_map_id) {
	// 	auto it = g_map_id_redirects.find(original_map_id);
	// 	if (it != g_map_id_redirects.end()) {
	// 		g_current_loading_map_id = it->second;
	// 		return it->second;
	// 	}
	//
	// 	g_current_loading_map_id = -1;
	// 	return original_map_id;
	// }

	// int area_resolve_id_for_city_match(int map_index) {
	//        auto it = g_map_id_to_original.find(map_index);
	//        if (it != g_map_id_to_original.end()) {
	//            logger.debug("Translating dynamic map ID {} back to original {} for worldmap area match", map_index, it->second);
	//            return it->second;
	//        }
	//        return map_index;
	//    }

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

		logger.debug("Redirecting asset path for map: {} -> {}", name, resolved_path);
		return resolved_path;
	}

	void area_on_map_header_set(fallout::MapHeader* header) {
		if (g_current_loading_map_id != -1 && header != nullptr) {
			logger.info("Memory patching map header index: {} -> {}", static_cast<int>(header->index), g_current_loading_map_id);

			header->index = fallout::Map(g_current_loading_map_id);
		}

		g_current_loading_map_id = -1;
	}

	int area_register_map(const CkAreaMapFFI& data) {
        std::string current_mod = ck_get_current_mod_id();
        std::string map_file_name = data.map_file;

        std::string map_file_upper = map_file_name;
        std::transform(map_file_upper.begin(), map_file_upper.end(), map_file_upper.begin(), ::toupper);
        std::string map_file_lower = map_file_name;
        std::transform(map_file_lower.begin(), map_file_lower.end(), map_file_lower.begin(), ::tolower);

        if (gMapPaths.find(map_file_lower) != gMapPaths.end()) {
            logger.error("Mod '{}' failed to register map: file '{}.MAP' is already in use by another map/location!",
                    current_mod, map_file_name);
            return -1;
        }

        std::string name          = data.name;
        std::string sub_name      = data.sub_name;
        std::string music         = data.music;
        std::string sfx           = data.sfx;

        int map_index = ck::config_maps::register_map(current_mod, map_file_name, name, music, sfx);
        if (map_index == -1) return -1;

        std::string map_file_path = std::format("../mods/{}/maps/{}.MAP", current_mod, map_file_upper);
        gMapPaths[map_file_lower]    = map_file_path;
        gRuntimeMaps[map_file_lower] = map_index;

        int map_base_id = map_index * 3;
        ck::messages_add_string("map.msg", map_base_id + 100, name);
        ck::messages_add_string("map.msg", map_base_id + 101, name);

        std::string full_description = sub_name.empty() ? name : sub_name;
        ck::messages_add_string("map.msg", map_base_id + 200, full_description);

		return map_index;
	}

	// int area_override_map(int original_map_id, const CkAreaMapFFI& data) {
	//        int map_id = ck::area_register_map(data);
	//        if (map_id == -1) return -1;
	//
	//        g_map_id_redirects[original_map_id] = map_id;
	// 	g_map_id_to_original[map_id]        = original_map_id;
	//
	//        logger.info("Redirect established: original ID {} -> {}", original_map_id, map_id);
	//        return map_id;
	//    }

	int area_register_location(const std::string& name, int world_x, int world_y, const std::string& size) {
		int location_index = ck::config_city::register_location(ck_get_current_mod_id(), name, world_x, world_y, size);
        if (location_index == -1) return -1;

        ck::messages_add_string("map.msg", 1500 + location_index, name);
		return location_index;
	}

	int expand_location(int area_id, const std::string& map_lookup_name, int x, int y) {
		int target_entrance_id = ck::config_city::expand_location(ck_get_current_mod_id(), area_id, map_lookup_name, x, y);

        int townmap_msg_id = 200 + (area_id * 10) + target_entrance_id;
        ck::messages_add_string("worldmap.msg", townmap_msg_id, map_lookup_name);

		return target_entrance_id;
    }
}

int ck_area_register_location(const char* name, int worldX, int worldY, const char* size) {
	return ck::area_register_location(name, worldX, worldY, size);
}

int ck_area_register_map(const CkAreaMapFFI* data) {
    if (!data) return -1;

    return ck::area_register_map(*data);
}

// int ck_area_override_map(int original_map_id, const CkAreaMapFFI* data) {
// 	if (!data) return -1;
//
//     return ck::area_override_map(original_map_id, *data);
// }

int ck_area_expand_location(int area_id, const char* map_lookup_name, int townmap_x, int townmap_y) {
	if (!map_lookup_name) return -1;

	return ck::expand_location(area_id, map_lookup_name, townmap_x, townmap_y);
}

