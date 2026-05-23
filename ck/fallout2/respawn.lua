-- ck/fallout2/respawn.lua

local respawn = {}

local gameTime = require('fallout2.game_time')

function respawn.markToday()
  return gameTime.getTotalDays()
end

function respawn.isReady(lastRespawnDay, days)
  return gameTime.hasDaysPassed(days, lastRespawnDay)
end

function respawn.try(area, callback)
  if not respawn.isReady(area.lastRespawnDay, area.respawnDays) then
    return
  end

  callback()

  area.lastRespawnDay = respawn.markToday()
end


return respawn
