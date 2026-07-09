local ffi = require('ffi')

ffi.cdef[[
typedef struct {
    const char* map_file;
    const char* name;
    const char* sub_name;
    const char* music;
} CkAreaMapFFI;

int ck_area_override_map(int map_id, const CkMapFFI* data);
]]

local locations = {}

function locations.override_map(map_id, config)
    assert(type(map_id) == "number", "map_id number required!")
    assert(config.file_name, "file_name is required!")
    assert(config.name,      "name is required!")

    local data = ffi.new("CkAreaMapFFI")

    data.map_id    = map_id
    data.file_name = config.file_name
    data.name      = config.name
    data.sub_name  = config.sub_name
    data.music     = config.music or "17arroyo"

    return ffi.C.ck_area_override_map(map_id, data)
end

return locations
