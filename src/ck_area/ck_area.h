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
    const char* area_resolve_path(const char* name);
    void        area_on_header_loaded(fallout::MapHeader* header);

	void override_map(int original_map_id, const CkMapFFI& data);
	int  register_map(const CkMapFFI& data);

    int register_area(const std::string& modId, const std::string& name,
                      int worldX, int worldY, const std::string& size,
                      const std::vector<std::string>& entranceLookups);
}

CK_API int  ck_area_override_map(int original_map_id, const CkMapFFI* data);
CK_API int  ck_area_register_map(const CkMapFFI* data);

CK_API int ck_area_register_area(const char* name,
		int world_x, int world_y, const char* size,
		const char** entrance_lookups, int entrance_count);
