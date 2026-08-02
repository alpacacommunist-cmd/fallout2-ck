local ffi = require('ffi')

local locations = {}

function locations.register(config)
    assert(config.name,   "name is required!")

    return ffi.C.ck_area_register_location(
        config.name,
        config.world_x,
        config.world_y,
        config.size or "small"
    )
end

function locations.expand(area_id, config)
    assert(type(area_id) == "number", "area_id number required!")
    assert(config.lookup_name, "lookup_name of the target map is required!")

    return ffi.C.ck_area_expand_location(
        area_id,
        config.lookup_name,
        config.townmap_x or 200,
        config.townmap_y or 200
    )
end

function locations.register_map(config)
  assert(config.map_file,  "map_file is required!")
  assert(config.name,      "name is required!")

  local data = ffi.new("CkAreaMapFFI")

  data.map_file  = config.map_file
  data.name      = config.name
  data.sub_name  = config.sub_name or ""
  data.music     = config.music or "17arroyo"
  data.sfx       = config.sfx   or "gntlwin1:25, gustwin1:5"

  return ffi.C.ck_area_register_map(data)
end

function locations.override_map(map_id, config)
    assert(type(map_id) == "number", "map_id number required!")
    assert(config.map_file,  "map_file is required!")
    assert(config.name,      "name is required!")

    local data = ffi.new("CkAreaMapFFI")

    data.map_file  = config.map_file
    data.name      = config.name
    data.sub_name  = config.sub_name
    data.music     = config.music or "17arroyo"

    return ffi.C.ck_area_override_map(map_id, data)
end

return locations
