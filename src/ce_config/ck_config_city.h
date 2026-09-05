#ifndef CK_CONFIG_CITY_H
#define CK_CONFIG_CITY_H

#include <string>

namespace ck::config_city {
    std::string format_section(int area_id);

    int next_index();
    bool preprocess_areas();

    int expand_location(const std::string& mod_id, int area_id,
                        const std::string& map_lookup_name, int x, int y);

	int register_location(const std::string& mod_id, const std::string& name,
			int world_x, int world_y, const std::string& size);
}

#endif
