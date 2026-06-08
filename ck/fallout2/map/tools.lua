-- ck/fallout2/map/tools.lua

local geometry = require('ck.fallout2.map.geometry')
local tools    = {}

function tools.spawnBrush(centerTile, radius, density, fids)
  local tiles = geometry.tilesInRadius(centerTile, radius)

  for _, tile in ipairs(tiles) do
    if tile ~= centerTile and math.random() <= density then
      local value = fids[math.random(#fids)]
      tools._applyValue(value, "scenery", false, tile)
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
      local dx   = x - 1

      local tx = ax - dx - math.floor((ay + dy) / 2) + math.floor(ay / 2)
      local ty = ay + dy

      if tx >= 0 and tx < w and ty >= 0 and ty < w then
        local tile = geometry.xyToTile(tx, ty)

        if char == " " then
          tools._onClear(tile)
        elseif mapping[char] then
          tools._applyElement(mapping[char], tile)
        end
      end
    end
  end
end

-- `tools` gets below functions from outside upon init
-- `tools` knows geometry, doesn't know bindings
function tools._applyElement(element, tile)
  local source = element.assets or element.fids
  if not source then return end

  local value = source[math.random(#source)]
  local mode  = element.mode or "place"

  tools._applyValue(value, element.type or "scenery", element.block or false, tile, mode)
end

function tools._applyValue(value, objType, block, tile, mode)
  -- overloaded in map/init.lua
end

function tools._onClear(tile)
  -- overloaded in map/init.lua
end

return tools
