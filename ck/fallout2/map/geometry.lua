-- ck/fallout2/map/geometry.lua
-- hex grid math

local geometry = {}

function geometry.grid_width()
  return 200
end

function geometry.tile_to_xy(tile)
  local w = geometry.grid_width()
  return w - 1 - (tile % w), math.floor(tile / w)
end

function geometry.xy_to_tile(x, y)
  local w = geometry.grid_width()
  return (w - 1 - x) + (y * w)
end

function geometry.is_valid(tile)
  local w = geometry.grid_width()
  return tile >= 0 and tile < (w * w)
end

function geometry.neighbour(tile, direction)
  local w  = geometry.grid_width()
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
function geometry.tiles_in_radius(center_tile, radius)
  local visited   = {}
  local queue     = {}
  local distances = {}
  local w = geometry.grid_width()

  visited[center_tile]   = true
  distances[center_tile] = 0
  table.insert(queue, center_tile)

  local head = 1
  while head <= #queue do
    local current = queue[head]
    head = head + 1

    if distances[current] < radius then
      for dir = 0, 5 do
        local nb = geometry.neighbour(current, dir)

        if geometry.is_valid(nb) then
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

function geometry.distance(tile_a, tile_b)
  local x1, y1 = geometry.tile_to_xy(tile_a)
  local x2, y2 = geometry.tile_to_xy(tile_b)

  local z1 = y1 - math.floor(x1 / 2)
  local z2 = y2 - math.floor(x2 / 2)

  local dx = x1 - x2
  local dz = z1 - z2
  local dy = (-x1 - z1) - (-x2 - z2)

  return math.max(math.abs(dx), math.abs(dy), math.abs(dz))
end

function geometry.tiles_in_rect(left, right, top, bottom)
  local tiles = {}
  for y = top, bottom do
    for x = left, right do
      if x >= 0 and x < 200 and y >= 0 and y < 200 then
        table.insert(tiles, geometry.xy_to_tile(x, y))
      end
    end
  end
  return tiles
end

function geometry.line(tile_a, tile_b)
  local dist = geometry.distance(tile_a, tile_b)
  local list = {}
  if dist == 0 then return {tile_a} end

  local x1, y1 = geometry.tile_to_xy(tile_a)
  local x2, y2 = geometry.tile_to_xy(tile_b)

  for i = 0, dist do
    local t = i / dist
    -- line interpolation xy
    local cx = math.floor(x1 + (x2 - x1) * t + 0.5)
    local cy = math.floor(y1 + (y2 - y1) * t + 0.5)
    table.insert(list, geometry.xy_to_tile(cx, cy))
  end
  return list
end

return geometry
