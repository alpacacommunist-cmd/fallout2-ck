#include "map.h"

#include "ce_config/ck_config_patch.h"
#include "ce_config/ck_message_patch.h"
#include "ck_area/ck_area.h"

#include <unordered_map>
#include <algorithm>
#include <format>

#include "ck_log.h"
static const Logger log("CK Locations");

namespace ck {

    struct MapRuntimeEntry {
        int map_index;
        std::string modId;
    };

	static std::unordered_map<int, int> gMapIdRedirects;
    static std::unordered_map<std::string, std::string> gMapPaths;        // key: lowercase map name -> full path
    static std::unordered_map<std::string, MapRuntimeEntry> gRuntimeMaps; // key: lowercase map name -> info
    static std::string gCurrentLoadingMapName = "";

    const char* area_resolve_path(const char* name) {
        if (name == nullptr) return nullptr;
        static char resolved_path[512];

        std::string fullName = name;
        size_t dot = fullName.find('.');

        std::string key = (dot != std::string::npos) ? fullName.substr(0, dot) : fullName;
        std::string ext = (dot != std::string::npos) ? fullName.substr(dot) : "";

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);

        if (!ext.empty() && ext != ".MAP")                        return nullptr;
        auto it = gMapPaths.find(key); if (it == gMapPaths.end()) return nullptr;

        gCurrentLoadingMapName = key;

        strncpy(resolved_path, it->second.c_str(), sizeof(resolved_path) - 1);
        resolved_path[sizeof(resolved_path) - 1] = '\0';

        return resolved_path;
    }

	void area_on_header_loaded(fallout::MapHeader* header) {
		if (gCurrentLoadingMapName.empty()) return;

		auto it = gRuntimeMaps.find(gCurrentLoadingMapName);
		if (it != gRuntimeMaps.end()) {
			log.info("Memory patch map index for '{}': {} -> {}",
					gCurrentLoadingMapName, header->index, it->second.map_index);

			header->index = it->second.map_index;
		}

		gCurrentLoadingMapName = "";
	}

	int area_register_map(const std::string& map_file_name, const std::string& name,
			const std::string& sub_name, const std::string& music) {

        static int next_map_index = -1;
        if (next_map_index == -1) {
            next_map_index = ck_config_next_map_index("data\\data\\maps.txt");
        }

        std::string map_file_upper = map_file;
        std::transform(map_file_upper.begin(), map_file_upper.end(), map_file_upper.begin(), ::toupper);
        std::string map_file_lower = map_file;
        std::transform(map_file_lower.begin(), map_file_lower.end(), map_file_lower.begin(), ::tolower);

        int map_index = next_map_index++;

        std::string map_file_path = std::format("../mods/{}/maps/{}.MAP", ck_get_current_mod_id(), map_file_upper);
        gMapPaths[map_file_lower] = map_file_path;
        gRuntimeMaps[map_file_lower] = { map_index, ck_get_current_mod_id() };

        log.info("Registered map: {} (ID: {}) -> {}", map_file_upper, map_index, map_file_path);

        std::string mapSection = "Map " + std::to_string(map_index);
        ck_config_patch_add("data\\maps.txt", mapSection, "lookup_name", name);
        ck_config_patch_add("data\\maps.txt", mapSection, "map_name",    map_file_upper);
        ck_config_patch_add("data\\maps.txt", mapSection, "music",       music);
        ck_config_patch_add("data\\maps.txt", mapSection, "saved",       "Yes");

        int mapMsgBase = (map_index * 3) + 100;
        ck_message_patch_add("game/map.msg", mapMsgBase, name);
        ck_message_patch_add("game/map.msg", mapMsgBase + 1, name);
        ck_message_patch_add("game/map.msg", (map_index * 3) + 200, subName);

        return map_index;
    }

    int area_override_map(int original_map_id, const CkAreaMapFFI& data) {
        std::string map_file  = data.map_file ? data.map_file  : std::string();
        std::string name      = data.name     ? data.name      : std::string();
        std::string sub_name  = data.sub_name ? data.sub_name  : std::string();
        std::string music     = data.music    ? data.music     : "17arroyo";

        int map_id = ck::area_register_map(map_file, name, sub_name, music);

        gMapIdRedirects[origonal_map_id] = map_id;

        log.info("Redirect established: original ID {} -> {}", original_map_id, map_id);

		return map_id;
    }

	int area_register_location(const std::string& modId, const std::string& name,
			int worldX, int worldY, const std::string& size,
			const std::vector<std::string>& entranceLookups) {

		static int nextAreaIdx = -1;
		if (nextAreaIdx == -1) {
			nextAreaIdx = ck_config_next_area_index("data\\data\\city.txt");
		}

		int areaIdx = nextAreaIdx++;

		std::string areaSection = "Area " + std::to_string(areaIdx);
		std::string worldPos = std::format("{},{}", worldX, worldY);

		log.info("Registered worldmap area: {} (ID: {})", name, areaIdx);

		ck_config_patch_add("data\\city.txt", areaSection, "area_name",             name);
		ck_config_patch_add("data\\city.txt", areaSection, "world_pos",             worldPos);
		ck_config_patch_add("data\\city.txt", areaSection, "start_state",           "On");
		ck_config_patch_add("data\\city.txt", areaSection, "size",                  size);
		ck_config_patch_add("data\\city.txt", areaSection, "townmap_art_idx",       "-1");
		ck_config_patch_add("data\\city.txt", areaSection, "townmap_label_art_idx", "-1");

		// entrances
		for (size_t i = 0; i < entranceLookups.size(); ++i) {
			std::string entrance = std::format("On,350,275,{},-1,-1,3", entranceLookups[i]);
			ck_config_patch_add("data\\city.txt", areaSection, "entrance_" + std::to_string(i), entrance);
		}

		// city name
		ck_message_patch_add("game/map.msg", 1500 + areaIdx, name);

		return areaIdx;
	}

}

int ck_area_override_map(int original_map_id, const CkAreaMapFFI* data) {
    return ck::area_override_map(original_map_id, *data);
}

int ck_area_register_map(const CkAreaMapFFI* data) {
    if (!data) return -1;

    return ck::register_map(
        data->map_file ? data->map_file : std::string(),
        data->name     ? data->name     : std::string(),
        data->sub_name ? data->sub_name : std::string(),
        data->music    ? data->music    : std::string("17arroyo")
    );
}

int ck_area_register_area(const char* modId, const char* name, int worldX, int worldY,
		const char* size, const char** entranceLookups, int entranceCount) {

	std::vector<std::string> lookups;
	for (int i = 0; i < entranceCount; ++i) lookups.push_back(entranceLookups[i]);

	return ck::register_area(modId, name, worldX, worldY, size, lookups);
}
