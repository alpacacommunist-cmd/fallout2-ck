-- ck/fallout2/map/geometry.lua
-- hex grid math

local geometry = {}

function geometry.gridWidth()
  return 200
end

function geometry.tileToXY(tile)
  local w = geometry.gridWidth()
  return w - 1 - (tile % w), math.floor(tile / w)
end

function geometry.xyToTile(x, y)
  local w = geometry.gridWidth()
  return (w - 1 - x) + (y * w)
end

function geometry.isValid(tile)
  local w = geometry.gridWidth()
  return tile >= 0 and tile < (w * w)
end

function geometry.neighbour(tile, direction)
  local w  = geometry.gridWidth()
  local tx = w - 1 - (tile % w)
  local odd = (tx % 2 ~= 0)

  if direction == 0 then return tile - w           end  -- NE
  if direction == 1 then return odd and (tile - w - 1) or (tile - w) end  -- N
  if direction == 2 then return odd and (tile - 1) or (tile + 1)     end  -- NW
  if direction == 3 then return tile + w           end  -- SW
  if direction == 4 then return odd and (tile + w) or (tile + w + 1) end  -- S
  if direction == 5 then return odd and (tile + 1) or (tile - 1)     end  -- SE
  return -1
end

-- BFS — all tiles in radius
function geometry.tilesInRadius(centerTile, radius)
  local visited   = {}
  local queue     = {}
  local distances = {}
  local w = geometry.gridWidth()

  visited[centerTile]   = true
  distances[centerTile] = 0
  table.insert(queue, centerTile)

  local head = 1
  while head <= #queue do
    local current = queue[head]
    head = head + 1

    if distances[current] < radius then
      for dir = 0, 5 do
        local nb = geometry.neighbour(current, dir)

        if geometry.isValid(nb) then
          local cx = w - 1 - (current % w)
          local nx = w - 1 - (nb % w)

          if math.abs(cx - nx) <= 1 and not visited[nb] then
            visited[nb]   = true
            distances[nb] = distances[current] + 1
            table.insert(queue, nb)
          end
        end
      end
    end
  end

  return queue
end

function geometry.distance(tileA, tileB)
  local x1, y1 = geometry.tileToXY(tileA)
  local x2, y2 = geometry.tileToXY(tileB)

  local z1 = y1 - math.floor(x1 / 2)
  local z2 = y2 - math.floor(x2 / 2)

  local dx = x1 - x2
  local dz = z1 - z2
  local dy = (-x1 - z1) - (-x2 - z2)

  return math.max(math.abs(dx), math.abs(dy), math.abs(dz))
end

function geometry.tilesInRect(left, right, top, bottom)
  local tiles = {}
  for y = top, bottom do
    for x = left, right do
      if x >= 0 and x < 200 and y >= 0 and y < 200 then
        table.insert(tiles, geometry.xyToTile(x, y))
      end
    end
  end
  return tiles
end

function geometry.line(tileA, tileB)
  local dist = geometry.distance(tileA, tileB)
  local list = {}
  if dist == 0 then return {tileA} end

  local x1, y1 = geometry.tileToXY(tileA)
  local x2, y2 = geometry.tileToXY(tileB)

  for i = 0, dist do
    local t = i / dist
    -- line interpolation xy
    local cx = math.floor(x1 + (x2 - x1) * t + 0.5)
    local cy = math.floor(y1 + (y2 - y1) * t + 0.5)
    table.insert(list, geometry.xyToTile(cx, cy))
  end
  return list
end

return geometry
