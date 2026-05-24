-- mods/game_time_extender/init.lua
print("[Mod] Loading Game Time Extender...")

local config = require('fallout2.config')
local log    = require('fallout2.log')
local events = require('fallout2.events')
local gameTime = require('fallout2.game_time')
local respawn = require('fallout2.respawn')

-- we're gona use a separate config module
-- engine doesn't do none of this yet, that's just front
config.set('max_game_time_years', 100)

events.on('onDayPassed', function()
  local date = gameTime.getDate()

  log.print(
    string.format(
      "Date: %d/%d/%d Hour: %d",
      date.day, date.month, date.year, date.hour
    )
  )

  log.print("Total days: " .. gameTime.getTotalDays())
  log.print("Time of day: " .. gameTime.getTimeOfDay())
  log.print("3 days passed since day 0: " .. tostring(gameTime.hasDaysPassed(3, 0)))
  log.print("Season: " .. gameTime.getSeason())
  log.print("Is it summer yet: " .. tostring(gameTime.isSeason("summer")))
  log.print("Day of week: " .. gameTime.getDayOfWeek())

  log.print("Is it night time: " .. tostring(gameTime.isNight()))
  log.print("Is it morning time: " .. tostring(gameTime.isMorning()))
  log.print("Is it day time: " .. tostring(gameTime.isDay()))
  log.print("Is it evening time: " .. tostring(gameTime.isEvening()))

  log.print("Respawn ready after 3 days since day 0: " .. tostring(respawn.isReady(0, 3)))
end)

events.on('onTimeAdvance', function(hours, minutes)
  log.print("[MOD Lua] Time Advanced on " .. tostring(hours) .. " h. and " .. tostring(minutes) .. " minutes")
end)
