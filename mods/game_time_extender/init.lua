-- mods/game_time_extender/init.lua
log.info("Loading Game Time Extender...")

local config    = require('ck.fallout2.config')
local monitor   = require('ck.fallout2.monitor')
local game_time = require('ck.fallout2.game_time')
local locations = require('ck.fallout2.locations')
local map       = require('ck.fallout2.map')

-- local new_hunting_grounds = locations.override_map(35,{
--   map_file = 'tstcv', name = "Hunting Grounds", sub_name = "", music = "07desert"
-- })
--
-- log.info("new_hunting_grounds id: " .. new_hunting_grounds)

new_location_id  = locations.register({ name = "Test Caves", world_x = 220, world_y = 140, size = "small" })
local test_caves_id = locations.register_map({
    map_file = 'tstcv',
    name     = "Secret Caves",
    sub_name = "Very secret",
    music    = "07desert"
})
locations.expand(new_location_id, { lookup_name = "Secret Caves" })

log.info('test caves id: ' .. tostring(test_caves_id))

events.on('onMapEnter', function()
  local map_id = map.get_id()

  if map_id ~= test_caves_id then return end

  if map.get_var(0) == 0 then
    monitor.print("first time here")

    map.set_var(0, 1)
  else
    monitor.print("already was here")
  end
end)

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
  log.info("Time Advanced on " .. tostring(hours) .. " h. and " .. tostring(minutes) .. " minutes")
end)
