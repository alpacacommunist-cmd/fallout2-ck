#ifndef CK_CONFIG_MAPS_H
#define CK_CONFIG_MAPS_H

#include <string>

namespace ck::config_maps {
    std::string format_section(int map_id);

    int get_next_index();
}

#endif
