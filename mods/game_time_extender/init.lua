-- mods/game_time_extender/init.lua
print("[Mod] Loading Game Time Extender...")

local config    = require('ck.fallout2.config')
local monitor   = require('ck.fallout2.monitor')
local game_time = require('ck.fallout2.game_time')

-- we're gona use a separate config module
-- engine doesn't do none of this yet, that's just front
config.set('max_game_time_years', 100)

events.on('onDayPassed', function()
  local date = game_time.get_date()

  monitor.print(
    string.format(
      "Date: %d/%d/%d Hour: %d",
      date.day, date.month, date.year, date.hour
    )
  )

  monitor.print("Total days: " .. game_time.get_total_days())
  monitor.print("Time of day: " .. game_time.get_time_of_day())
  monitor.print("3 days passed since day 0: " .. tostring(game_time.has_days_passed(3, 0)))
  monitor.print("Season: " .. game_time.get_season())
  monitor.print("Is it summer yet: " .. tostring(game_time.is_season("summer")))
  monitor.print("Day of week: " .. game_time.get_day_of_week())

  monitor.print("Is it night time: " .. tostring(game_time.is_night()))
  monitor.print("Is it morning time: " .. tostring(game_time.is_morning()))
  monitor.print("Is it day time: " .. tostring(game_time.is_day()))
  monitor.print("Is it evening time: " .. tostring(game_time.is_evening()))
end)

events.on('onTimeAdvance', function(hours, minutes)
  log.print("[MOD Lua] Time Advanced on " .. tostring(hours) .. " h. and " .. tostring(minutes) .. " minutes")
end)
