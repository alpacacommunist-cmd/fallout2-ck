local geometry = require('ck.fallout2.map.geometry')
local tools    = {}
local map_ref  = nil

function tools.init(map_instance)
  map_ref = map_instance
end

function tools.spawnBrush(centerTile, radius, density, pool, config)
  local tiles = geometry.tilesInRadius(centerTile, radius)
  for _, tile in ipairs(tiles) do
    if tile ~= centerTile and math.random() <= density then
      local value = pool[math.random(#pool)]
      map_ref.place(value, tile, config)
    end
  end
end

function tools.spawnMask(anchorTile, maskTable, mapping)
  local w = geometry.gridWidth()
  local ax, ay = geometry.tileToXY(anchorTile)

  for y, row in ipairs(maskTable) do
    local dy = y - 1
    for x = 1, #row do
      local char = row:sub(x, x)
      local dx = x - 1

      local tx = ax - dx - math.floor((ay + dy) / 2) + math.floor(ay / 2)
      local ty = ay + dy

      if tx >= 0 and tx < w and ty >= 0 and ty < w then
        local tile = geometry.xyToTile(tx, ty)

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
