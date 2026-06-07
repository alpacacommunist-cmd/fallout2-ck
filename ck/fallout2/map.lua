-- ck/fallout2/map.lua
--

local map = {}
local tools = {}

-- it's square bro fucc
local HEX_DIRECTIONS = {
  [0] = -200, -- up         -> screen: top-right (North-East)
  [1] = -199, -- up-left    -> screen: up (North)
  [2] = 1,    -- left       -> screen: top-left (North-West)
  [3] = 200,  -- down       -> screen: down-left (South-West)
  [4] = 199,  -- down-right -> screen: down (South)
  [5] = -1    -- right      -> screen: down-right (South-East)
}

function tools.spawnBrush(centerTile, radius, density, fids)
  local gridWidth = 200
  -- do it bro
  if fallout and fallout.tileGetHexGridWidth then
    gridWidth = fallout.tileGetHexGridWidth()
  end

  local centerX = gridWidth - 1 - (centerTile % gridWidth)
  local centerY = math.floor(centerTile / gridWidth)

  for y = centerY - radius, centerY + radius do
    for x = centerX - radius, centerX + radius do

      local dx = x - centerX
      local dy = y - centerY

      local distance = math.max(math.abs(dx), math.abs(dy), math.abs(dx + dy))

      if distance <= radius and (dx ~= 0 or dy ~= 0) then
        if math.random() <= density then

          local targetTile = (gridWidth - 1 - x) + (y * gridWidth)
          if x >= 0 and x < gridWidth and targetTile >= 0 and targetTile < 40000 then
            local randomFid = fids[math.random(#fids)]

            if map.createObject then
              map.createObject(randomFid, targetTile)
            else
              map.addScenery(randomFid, targetTile)
            end
          end

        end
      end

    end
  end
end

function tools.spawnMask(anchorTile, maskTable, mapping)
  local gridWidth = 200
  if fallout and fallout.tileGetHexGridWidth then
    gridWidth = fallout.tileGetHexGridWidth()
  end

  local anchorX = gridWidth - 1 - (anchorTile % gridWidth)
  local anchorY = math.floor(anchorTile / gridWidth)

  for y, row in ipairs(maskTable) do
    local dy = y - 1

    for x = 1, #row do
      local char = row:sub(x, x)
      local dx = x - 1

      local targetX = anchorX - dx
      local targetY = anchorY + dy

      local targetTile = (gridWidth - 1 - targetX) + (targetY * gridWidth)

      if targetX >= 0 and targetX < gridWidth and targetTile >= 0 and targetTile < 40000 then
        if char == " " then
          if map.removeBlocker then map.removeBlocker(targetTile) end

        elseif mapping[char] then
          local element = mapping[char]
          local randomFid = element.fids[math.random(#element.fids)]

          if element.type == "tile" then
            if map.addTile then map.addTile(randomFid, targetTile) end
          else
            map.addScenery(randomFid, targetTile)
          end

          if element.block then map.createBlocker(targetTile) end
        end

      end
    end
  end
end

function map.spawnCritter(pid)
  return ckSpawnCritter(pid)
end

function map.getId()
  return ck.map.get_id()
end

function map.addScenery(fid, tile)
  ck.map.add_scenery(fid, tile)
end

function map.addTile(fid, tile)
  ck.map.add_tile(fid, tile)
end

function map.setBorders(left, right, top, bottom)
  ck.map.set_camera_borders(left, right, top, bottom)
end

function map.removeBlocker(tile)
  ck.map.remove_blocker(tile)
end

function map.createBlocker(tile)
  ck.map.create_blocker(tile)
end

function map.createObject(fid, tile)
  ck.map.create_object(fid, tile)
end

map.tools = tools
return map
