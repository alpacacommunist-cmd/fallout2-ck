#include "map/ck_map.h"
#include "map/ck_map_camera_borders.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

#include "ck_log.h"
static const Logger log("CK Camera Borders");

extern "C" const char* ck_get_current_mod_id();

namespace {
    //  map_id -> coordinates
    std::unordered_map<int, CkCameraBorders> g_camera_borders;

    // mod_id -> map_id
    std::unordered_map<std::string, std::vector<int>> g_mod_camera_borders;
}

namespace ck::map::borders {
    void clear() {
		g_camera_borders.clear();
		g_mod_camera_borders.clear();

        log.info("Camera borders registry cleared.");
    }

    void clear_for_mod(const char* mod_id) {
		if (mod_id == nullptr) return;
		std::string mod_str(mod_id);

		auto it = g_mod_camera_borders.find(mod_str);
		if (it != g_mod_camera_borders.end()) {
			for (int map_id : it->second) {
				g_camera_borders.erase(map_id);
			}

			g_mod_camera_borders.erase(it);
			log.info("Hot Reload: Cleared custom camera borders for mod '{}'", mod_str);
		}
    }

    bool has_borders_for_map(int map_id) {
        return g_camera_borders.find(map_id) != g_camera_borders.end();
    }

    bool is_camera_position_allowed(int tile) {
        int map_id = ck_map_get_id();

        auto it = g_camera_borders.find(map_id);
        if (it == g_camera_borders.end()) {
            return true;
        }

        int grid_width = fallout::tileGetHexGridWidth();
        int tile_x = grid_width - 1 - tile % grid_width;
        int tile_y = tile / grid_width;

        const auto& bounds = it->second;
        return tile_x >= bounds.left && tile_x <= bounds.right &&
               tile_y >= bounds.top  && tile_y <= bounds.bottom;
    }
}

bool ck_map_has_camera_borders(int map_id) {
	return ck::map::borders::has_borders_for_map(map_id);
}

void ck_map_set_camera_borders(int map_id, const CkCameraBorders* borders) {
	if (!borders) return;

	g_camera_borders[map_id] = *borders;

	std::string mod_str(ck_get_current_mod_id());

	auto& map_list = g_mod_camera_borders[mod_str];
	if (std::find(map_list.begin(), map_list.end(), map_id) == map_list.end()) {
		map_list.push_back(map_id);
	}

	log.debug("Registered borders for map {} (Mod: {}): L:{}, R:{}, T:{}, B:{}",
			map_id, mod_str,
			borders->left, borders->right, borders->top, borders->bottom);

	fallout::mapEdgeFree();
}

void ck_map_clear_camera_borders() {
	ck::map::borders::clear();
}

void ck_map_clear_camera_borders_for_mod(const char* mod_id) {
	ck::map::borders::clear_for_mod(mod_id);
}

CkCameraBorders ck_map_get_camera_borders(int map_id) {
	auto it = g_camera_borders.find(map_id);
	if (it != g_camera_borders.end()) {
		return it->second;
	}

	return CkCameraBorders{};
}
