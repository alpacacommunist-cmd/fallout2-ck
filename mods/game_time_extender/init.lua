-- mods/game_time_extender/init.lua
print("[Mod] Loading Game Time Extender...")

local config = require('ck.fallout2.config')
local log    = require('ck.fallout2.log')
local events = require('ck.fallout2.events')
local game_time = require('ck.fallout2.game_time')
local respawn = require('ck.fallout2.respawn')

-- we're gona use a separate config module
-- engine doesn't do none of this yet, that's just front
config.set('max_game_time_years', 100)

events.on('onDayPassed', function()
  local date = game_time.get_date()

  log.print(
    string.format(
      "Date: %d/%d/%d Hour: %d",
      date.day, date.month, date.year, date.hour
    )
  )

  log.print("Total days: " .. game_time.get_total_days())
  log.print("Time of day: " .. game_time.get_time_of_day())
  log.print("3 days passed since day 0: " .. tostring(game_time.has_days_passed(3, 0)))
  log.print("Season: " .. game_time.get_season())
  log.print("Is it summer yet: " .. tostring(game_time.is_season("summer")))
  log.print("Day of week: " .. game_time.get_day_of_week())

  log.print("Is it night time: " .. tostring(game_time.is_night()))
  log.print("Is it morning time: " .. tostring(game_time.is_morning()))
  log.print("Is it day time: " .. tostring(game_time.is_day()))
  log.print("Is it evening time: " .. tostring(game_time.is_evening()))

  log.print("Respawn ready after 3 days since day 0: " .. tostring(respawn.is_ready(0, 3)))
end)

events.on('onTimeAdvance', function(hours, minutes)
  log.print("[MOD Lua] Time Advanced on " .. tostring(hours) .. " h. and " .. tostring(minutes) .. " minutes")
end)
