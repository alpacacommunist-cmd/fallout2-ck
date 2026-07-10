#include "ce_config/ck_config_patch.h"
#include "ce_config/ck_config_maps.h"
#include "db.h"
#include <format>
#include <string_view>
#include <charconv>

#include "ck_log.h"
static const Logger log("CK Maps Config");

namespace ck::config_maps {

    std::string format_section(int map_id) {
        return std::format("Map {:03d}", map_id);
    }

    int get_next_index() {
		return ck::config_find_next_free_index_vfs("data\\data\\maps.txt", "Map");
    }

}
