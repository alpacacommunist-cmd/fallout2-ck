-- ck/fallout2/respawn.lua

local respawn = {}

local game_time = require('ck.fallout2.game_time')

function respawn.mark_today()
  return game_time.get_total_days()
end

function respawn.is_ready(last_respawn_day, days)
  return game_time.has_days_passed(days, last_respawn_day)
end

function respawn.try(area, callback)
  if not respawn.is_ready(area.last_respawn_day, area.respawn_days) then
    return
  end

  callback()

  area.last_respawn_day = respawn.mark_today()
end


return respawn
