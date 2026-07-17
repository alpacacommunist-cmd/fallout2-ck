local ffi = require("ffi")

local C = ffi.C

local exit_grid = {}

exit_grid.first_pid = C.ck_proto_first_exit_grid_pid
exit_grid.last_pid  = C.ck_proto_last_exit_grid_pid

exit_grid.styles = {
    FIRST = exit_grid.first_pid(),
    LAST  = exit_grid.last_pid(),

    STYLE_0 = exit_grid.first_pid() + 0,
    STYLE_1 = exit_grid.first_pid() + 1,
    STYLE_2 = exit_grid.first_pid() + 2,
    STYLE_3 = exit_grid.first_pid() + 3,
    STYLE_4 = exit_grid.first_pid() + 4,
    STYLE_5 = exit_grid.first_pid() + 5,
    STYLE_6 = exit_grid.first_pid() + 6,
    STYLE_7 = exit_grid.first_pid() + 7
}

exit_grid.create_in_rect  = C.ck_landscape_create_exit_grid_in_rect
exit_grid.create_at_tile  = C.ck_landscape_create_exit_grid_at_tile
exit_grid.destroy_in_rect = C.ck_landscape_destroy_exit_grid_in_rect

function exit_grid.spawn_in_rect(t1, t2, t3, t4, config)
    -- config : { map = 10, tile = 15000, elevation = 0, rotation = 1, style = 0 }

    local grid_data = ffi.new("CKExitGridData")
    grid_data.target_map       = config.map
    grid_data.target_tile      = config.tile
    grid_data.target_elevation = config.elevation or 0
    grid_data.target_rotation  = config.rotation or 0

    local style_offset = config.style or 0
    local final_pid    = exit_grid.styles.FIRST + style_offset

    exit_grid.create_in_rect(t1, t2, t3, t4, final_pid, grid_data)
end

function exit_grid.spawn_in_line(t1, t2, config, thickness)
  -- config : { map = 10, tile = 15000, elevation = 0, rotation = 1, style = 0 }

  local grid_data = ffi.new("CKExitGridData")
  grid_data.target_map       = config.map
  grid_data.target_tile      = config.tile
  grid_data.target_elevation = config.elevation or 0
  grid_data.target_rotation  = config.rotation or 0

  local style_offset = config.style or 0
  local final_pid    = exit_grid.styles.FIRST + style_offset
  thickness = thickness or 1

  -- middle line
  local geometry = require('ck.fallout2.map.geometry')
  local core_line = geometry.line(t1, t2)

  -- thickness brush
  local final_tiles = {}

  for _, tile in ipairs(core_line) do
    if thickness > 1 then
      local brush_tiles = geometry.tiles_in_radius(tile, thickness - 1)
      for _, brush_tile in ipairs(brush_tiles) do
        final_tiles[brush_tile] = true
      end
    else
      final_tiles[tile] = true
    end
  end

  for tile_to_spawn, _ in pairs(final_tiles) do
    ffi.C.ck_landscape_create_exit_grid_at_tile(tile_to_spawn, final_pid, grid_data)
  end
end

return exit_grid
