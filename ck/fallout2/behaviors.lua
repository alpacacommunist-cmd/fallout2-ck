-- ck/fallout2/behaviors.lua
local geometry = require('ck.fallout2.map.geometry')
local behaviors = {}

function behaviors.wander(radius)
  local home_tile = nil

  return function(critter, current_ticks)
    if not home_tile then
      home_tile = critter:tile()
    end

    if math.random(1, 100) > 30 then
      return
    end

    local available_tiles = geometry.tilesInRadius(home_tile, radius)
    if #available_tiles > 1 then
      local target_tile = available_tiles[math.random(1, #available_tiles)]

      critter._is_moving = true
      critter:animate():walk_to(target_tile):submit()
      critter._is_moving = false
    end
  end
end

function behaviors.patrol(waypoints_table, delay_seconds)
  local current_idx = 1
  local ticks_to_wait = 0
  local interval_ticks = delay_seconds * 10

  return function(critter, current_ticks)
    if current_ticks < ticks_to_wait then
      return
    end

    local critter_tile = critter:tile()
    local target_tile = waypoints_table[current_idx]

    if critter_tile == target_tile then
      critter._is_moving = false

      current_idx = current_idx + 1
      if current_idx > #waypoints_table then
        current_idx = 1
      end

      ticks_to_wait = current_ticks + interval_ticks
      return
    end

    if critter._is_moving then return end

    critter._is_moving = true
    critter:animate():walk_to(target_tile):submit()
  end
end

return behaviors
