local geometry = require('ck.fallout2.map.geometry')
local tools    = {}
local map_ref  = nil

function tools.init(map_instance)
  map_ref = map_instance
end

function tools.spawn_brush(center_tile, radius, density, pool, config)
  local tiles = geometry.tiles_in_radius(center_tile, radius)
  for _, tile in ipairs(tiles) do
    if tile ~= center_tile and math.random() <= density then
      local value = pool[math.random(#pool)]
      map_ref.place(value, tile, config)
    end
  end
end

function tools.spawn_mask(anchor_tile, mask_table, mapping)
  local w = geometry.grid_width()
  local ax, ay = geometry.tile_to_xy(anchor_tile)

  for y, row in ipairs(mask_table) do
    local dy = y - 1
    for x = 1, #row do
      local char = row:sub(x, x)
      local dx = x - 1

      local tx = ax - dx - math.floor((ay + dy) / 2) + math.floor(ay / 2)
      local ty = ay + dy

      if tx >= 0 and tx < w and ty >= 0 and ty < w then
        local tile = geometry.xy_to_tile(tx, ty)

        if char == " " then
          map_ref.remove_blocker(tile)
        elseif mapping[char] then
          local element = mapping[char]
          local pool = element.assets or element.fids

          if pool then
            local value = pool[math.random(#pool)]
            map_ref.place(value, tile, element)
          end
        end

      end
    end
  end
end

return tools
