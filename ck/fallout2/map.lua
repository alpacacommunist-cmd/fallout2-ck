-- ck/fallout2/map.lua
--

local map = {}
local tools = {}

local HEX_DIRECTIONS = {
  [0] = -200, -- top-left
  [1] = -199, -- top-right
  [2] = 1,    -- right
  [3] = 200,  -- down-right
  [4] = 199,  -- down-left
  [5] = -1    -- left
}

function tools.spawnBrush(centerTile, radius, density, fids)
  local visited = {}
  local queue = { {tile = centerTile, dist = 0} }
  visited[centerTile] = true

  while #queue > 0 do
    local current = table.remove(queue, 1)

    if current.tile ~= centerTile and math.random() <= density then
      local random_fid = fids[math.random(#fids)]
      -- map.addScenery(random_fid, current.tile)
      map.createObject(random_fid, current.tile)
    end

    if current.dist < radius then
      for i = 0, 5 do
        local nextTile = current.tile + HEX_DIRECTIONS[i]

        if nextTile >= 0 and nextTile < 40000 and not visited[nextTile] then
          visited[nextTile] = true
          table.insert(queue, {tile = nextTile, dist = current.dist + 1})
        end
      end
    end
  end
end

function tools.spawnMask(anchorTile, maskTable, mapping)
  for y, row in ipairs(maskTable) do
    for x = 1, #row do
      local char = row:sub(x, x)

      if mapping[char] then
        local targetTile = anchorTile + (x - 1) * 1 + (y - 1) * 200

        if targetTile >= 0 and targetTile < 40000 then
          local fidsPool = mapping[char]
          local randomFid = fidsPool[math.random(#fidsPool)]
          -- map.addScenery(random_fid, targetTile)
          map.createObject(random_fid, targetTile)
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
