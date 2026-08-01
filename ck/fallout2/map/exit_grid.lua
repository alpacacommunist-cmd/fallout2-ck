local ffi = require("ffi")
local geometry = require('ck.fallout2.map.geometry')

local C = ffi.C

local exit_grid = {}

exit_grid.first_pid = C.ck_proto_first_exit_grid_pid
exit_grid.last_pid  = C.ck_proto_last_exit_grid_pid

exit_grid.styles = {
    FIRST = exit_grid.first_pid(),
    LAST  = exit_grid.last_pid(),

    STYLE_0 = exit_grid.first_pid() + 0, -- east
    STYLE_1 = exit_grid.first_pid() + 1, -- west
    STYLE_2 = exit_grid.first_pid() + 2, -- north
    STYLE_3 = exit_grid.first_pid() + 3, -- south
    STYLE_4 = exit_grid.first_pid() + 4,
    STYLE_5 = exit_grid.first_pid() + 5,
    STYLE_6 = exit_grid.first_pid() + 6,
    STYLE_7 = exit_grid.first_pid() + 7,
}

exit_grid.style_shifts = {
    [0] = 0, [1] = 3, [2] = 2, [3] = 5,
    [4] = 0, [5] = 4, [6] = 1, [7] = 3
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

function exit_grid.spawn_at_tile(tile, config)
  -- config : { map = 10, tile = 15000, elevation = 0, rotation = 1, style = 0 }

  local grid_data = ffi.new("CKExitGridData")
  grid_data.target_map       = config.map
  grid_data.target_tile      = config.tile
  grid_data.target_elevation = config.elevation or 0
  grid_data.target_rotation  = config.rotation or 0

  local style_offset = config.style or 0
  local final_pid    = exit_grid.styles.FIRST + style_offset

  exit_grid.create_at_tile(tile, final_pid, grid_data)
end

function exit_grid.spawn_in_line(t1, t2, config, thickness)
  -- config : { map = 10, tile = 15000, elevation = 0, rotation = 1, style = 0 }
  thickness = thickness or 1

  local grid_data = ffi.new("CKExitGridData")
  grid_data.target_map       = config.map
  grid_data.target_tile      = config.tile
  grid_data.target_elevation = config.elevation or 0
  grid_data.target_rotation  = config.rotation or 0

  local style_offset = config.style or 0
  local final_pid    = exit_grid.styles.FIRST + style_offset

  local shift_direction = exit_grid.style_shifts[style_offset] or 0

  local spawned_tiles = {}
  local core_line = geometry.line(t1, t2)

  local SPRITE_TAIL_LENGTH = 4

  for layer = 0, thickness - 1 do
    for _, base_tile in ipairs(core_line) do

      local target_tile = base_tile
      if layer > 0 then
        local total_steps = layer * SPRITE_TAIL_LENGTH
        for step = 1, total_steps do
          target_tile = geometry.neighbour(target_tile, shift_direction)
        end
      end

      if geometry.is_valid(target_tile) and not spawned_tiles[target_tile] then
        exit_grid.create_at_tile(target_tile, final_pid, grid_data)
        spawned_tiles[target_tile] = true
      end
    end
  end
end

return exit_grid
