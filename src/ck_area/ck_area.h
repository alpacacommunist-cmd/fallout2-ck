#include "ck_api.h"

#include <string>
#include <vector>

namespace fallout {
    struct MapHeader;
}

namespace ck {
    const char* area_resolve_path(const char* name);
    void        area_on_header_loaded(fallout::MapHeader* header);

    int register_map(const std::string& modId, const std::string& mapsDir,
                     const std::string& mapFile, const std::string& name,
                     const std::string& subName, const std::string& music);

    int register_area(const std::string& modId, const std::string& name,
                      int worldX, int worldY, const std::string& size,
                      const std::vector<std::string>& entranceLookups);
}

CK_API int ck_area_register_map(const char* modId, const char* mapsDir,
		const char* mapFile, const char* name,
		const char* subName, const char* music);

CK_API int ck_area_register_area(const char* modId, const char* name,
		int worldX, int worldY, const char* size,
		const char** entranceLookups, int entranceCount);
