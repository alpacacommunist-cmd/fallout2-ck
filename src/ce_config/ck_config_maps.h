#ifndef CK_CONFIG_MAPS_H
#define CK_CONFIG_MAPS_H

#include <string>

namespace ck::config_maps {
    std::string format_section(int map_id);

    int get_next_index();

	int register_map(const std::string& mod_id, const std::string& map_file_name,
			const std::string& name, const std::string& music);
}

#endif
