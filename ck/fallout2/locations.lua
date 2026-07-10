local ffi = require('ffi')

ffi.cdef[[
typedef struct {
    const char* map_file;
    const char* name;
    const char* sub_name;
    const char* music;
} CkAreaMapFFI;

int ck_area_expand_location(int area_id, const char* custom_map_lookup_name, int townmap_x, int townmap_y);
int ck_area_override_map(int map_id, const CkAreaMapFFI* data);
]]

local locations = {}

function locations.override_map(map_id, config)
    assert(type(map_id) == "number", "map_id number required!")
    assert(config.map_file,  "file_name is required!")
    assert(config.name,      "name is required!")

    local data = ffi.new("CkAreaMapFFI")

    data.map_file  = config.map_file
    data.name      = config.name
    data.sub_name  = config.sub_name
    data.music     = config.music or "17arroyo"

    return ffi.C.ck_area_override_map(map_id, data)
end

function locations.expand_location(area_id, config)
    assert(type(area_id) == "number", "area_id number required!")
    assert(config.lookup_name, "lookup_name of the target map is required!")

    return ffi.C.ck_area_expand_location(
        area_id,
        config.lookup_name,
        config.townmap_x or 200,
        config.townmap_y or 200
    )
end

return locations
