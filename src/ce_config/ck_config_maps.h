#ifndef CK_CONFIG_MAPS_H
#define CK_CONFIG_MAPS_H

#include "ck_api.h"
#include <string>

namespace ck::config_maps {
    std::string format_section(int map_id);

    int next_index();
    bool preprocess_maps();

	int register_map(const std::string& mod_id, const std::string& map_file_name,
			const std::string& name, const std::string& music, const std::string& sfx);

    bool is_map_savable(int index);
}

CK_API bool ck_config_is_map_savable(int index);

#endif
