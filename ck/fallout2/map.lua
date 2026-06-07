-- ck/fallout2/map.lua
--

local map = {}
local tools = {}

function map.getNeighbourTile(tile, direction)
  local gridWidth = 200
  if fallout and fallout.tileGetHexGridWidth then
    gridWidth = fallout.tileGetHexGridWidth()
  end

  local tileX = gridWidth - 1 - (tile % gridWidth)
  local isOddX = (tileX % 2 ~= 0)

  if direction == 0 then     -- North-East
    return tile - gridWidth
  elseif direction == 1 then -- North
    return isOddX and (tile - gridWidth - 1) or (tile - gridWidth)
  elseif direction == 2 then -- North-West
    return isOddX and (tile - 1) or (tile + 1)
  elseif direction == 3 then -- South-West
    return tile + gridWidth
  elseif direction == 4 then -- South
    return isOddX and (tile + gridWidth) or (tile + gridWidth + 1)
  elseif direction == 5 then -- South-East
    return isOddX and (tile + 1) or (tile - 1)
  end
  return -1
end

-- 0(N)
function tools.spawnBrush(centerTile, radius, density, fids)
  local gridWidth = 200
  if fallout and fallout.tileGetHexGridWidth then
    gridWidth = fallout.tileGetHexGridWidth()
  end

  local visited = {}
  local queue = {}
  local distances = {}

  visited[centerTile] = true
  distances[centerTile] = 0
  table.insert(queue, centerTile)

  local head = 1

  while head <= #queue do
    local currentTile = queue[head]
    head = head + 1

    local currentDist = distances[currentTile]

    if currentDist < radius then
      for dir = 0, 5 do
        local neighbour = map.getNeighbourTile(currentTile, dir)

        if neighbour >= 0 and neighbour < (gridWidth * gridWidth) then
          local currX = gridWidth - 1 - (currentTile % gridWidth)
          local neighX = gridWidth - 1 - (neighbour % gridWidth)

          if math.abs(currX - neighX) <= 1 and not visited[neighbour] then
            visited[neighbour] = true
            distances[neighbour] = currentDist + 1
            table.insert(queue, neighbour)
          end
        end
      end
    end
  end

  for _, targetTile in ipairs(queue) do
    if targetTile ~= centerTile then
      if math.random() <= density then
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

-- O(w*h)
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

      local targetX = anchorX - dx - math.floor((anchorY + dy) / 2) + math.floor(anchorY / 2)
      local targetY = anchorY + dy

      local targetTile = (gridWidth - 1 - targetX) + (targetY * gridWidth)

      if targetX >= 0 and targetX < gridWidth and targetY >= 0 and targetY < gridWidth then
        if targetTile >= 0 and targetTile < (gridWidth * gridWidth) then

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
