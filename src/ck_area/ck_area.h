#include "ck_api.h"

#include <string>
#include <vector>

extern "C" const char* ck_get_current_mod_id();

namespace fallout {
    struct MapHeader;
}

struct CkAreaMapFFI {
    const char* map_file;
    const char* name;
    const char* sub_name;
    const char* music;
};

namespace ck {
	int         area_resolve_map_id(int original_map_id);
	int         area_resolve_id_for_city_match(int map_index);
    const char* area_resolve_path(const char* name);
    void        area_on_map_header_set(fallout::MapHeader* header);

	void override_map(int original_map_id, const CkAreaMapFFI& data);
	int  register_map(const CkAreaMapFFI& data);

    int register_area(const std::string& modId, const std::string& name,
                      int worldX, int worldY, const std::string& size,
                      const std::vector<std::string>& entranceLookups);
}

CK_API int ck_area_register_location(const char* name, int worldX, int worldY, const char* size);
CK_API int ck_area_expand_location(int area_id, const char* custom_map_lookup_name, int townmap_x, int townmap_y);
CK_API int ck_area_override_map(int original_map_id, const CkAreaMapFFI* data);
CK_API int ck_area_register_map(const CkAreaMapFFI* data);

CK_API int ck_area_register_area(const char* name,
		int world_x, int world_y, const char* size,
		const char** entrance_lookups, int entrance_count);
