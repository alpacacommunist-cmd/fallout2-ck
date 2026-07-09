#include "map.h"

#include "locations/ck_locations.h"

#include <unordered_map>
#include <algorithm>
#include <format>

#include "ck_log.h"
static const Logger log("CK Locations");

namespace ck {

    struct MapRuntimeEntry {
        int mapIdx;
        std::string modId;
    };

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

        if (!ext.empty() && ext != ".MAP") {
            return nullptr;
        }

        auto it = gMapPaths.find(key);
        if (it == gMapPaths.end()) return nullptr;

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
					gCurrentLoadingMapName, header->index, it->second.mapIdx);

			header->index = it->second.mapIdx;
		}

		gCurrentLoadingMapName = "";
	}

	int register_map(const std::string& modId, const std::string& mapsDir,
			const std::string& mapFile, const std::string& name,
			const std::string& subName, const std::string& music) {

		static int nextMapIdx = -1;
		if (nextMapIdx == -1) {
			nextMapIdx = ck_config_next_map_index("data\\data\\maps.txt");
		}

		std::string mapFileUpper = mapFile;
		std::transform(mapFileUpper.begin(), mapFileUpper.end(), mapFileUpper.begin(), ::toupper);
		std::string mapFileLower = mapFile;
		std::transform(mapFileLower.begin(), mapFileLower.end(), mapFileLower.begin(), ::tolower);

		int mapIdx = nextMapIdx++;

		std::string mapFilePath = std::format("../{}/{}.MAP", mapsDir, mapFileUpper);
		gMapPaths[mapFileLower] = mapFilePath;
		gRuntimeMaps[mapFileLower] = { mapIdx, modId };

		log.info("Registered map: {} (ID: {}) -> {}", mapFileUpper, mapIdx, mapFilePath);

		std::string mapSection = "Map " + std::to_string(mapIdx);
		ck_config_patch_add("data\\maps.txt", mapSection, "lookup_name", name);
		ck_config_patch_add("data\\maps.txt", mapSection, "map_name",    mapFileUpper);
		ck_config_patch_add("data\\maps.txt", mapSection, "music",       music);
		ck_config_patch_add("data\\maps.txt", mapSection, "saved",       "Yes");

		int mapMsgBase = (mapIdx * 3) + 100;
		ck_message_patch_add("game/map.msg", mapMsgBase, name);
		ck_message_patch_add("game/map.msg", mapMsgBase + 1, name);
		ck_message_patch_add("game/map.msg", (mapIdx * 3) + 200, subName);

		return mapIdx;
	}

	int register_area(const std::string& modId, const std::string& name,
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

int ck_area_register_map(const char* modId, const char* mapsDir, const char* mapFile,
		const char* name, const char* subName, const char* music) {
	return ck::register_map(modId, mapsDir, mapFile, name, subName, music);
}

int ck_area_register_area(const char* modId, const char* name, int worldX, int worldY,
		const char* size, const char** entranceLookups, int entranceCount) {

	std::vector<std::string> lookups;
	for (int i = 0; i < entranceCount; ++i) lookups.push_back(entranceLookups[i]);

	return ck::register_area(modId, name, worldX, worldY, size, lookups);
}
