-- mods/game_time_extender/init.lua
log.info("Loading Game Time Extender...")

local config    = require('ck.fallout2.config')
local monitor   = require('ck.fallout2.monitor')
local game_time = require('ck.fallout2.game_time')
local locations = require('ck.fallout2.locations')
local map       = require('ck.fallout2.map')

new_location_id  = locations.register({ name = "Test Caves", world_x = 220, world_y = 140, size = "small" })
local test_caves_id = locations.register_map({
    map_file = 'tstcv',
    name     = "Secret Caves",
    sub_name = "Very secret",
    music    = "07desert"
})
locations.expand(new_location_id, { lookup_name = "Secret Caves" })
log.info(string.format('test_caves_id: (area) %d -> map_id %d', new_location_id, test_caves_id))
--
local new_map_id = locations.register_map({
    map_file = 'tstcv2',
    name     = "Secret Hunting Grounds",
    sub_name = "Arroyo",
    music    = "07desert",
    sfx      = "gntlwin1:25, gntlwind:25, dogbark:20, dogbark1:20, gustwind:5, gustwin1:5"
})

entrance_id = locations.expand(0, {
    lookup_name = "Secret Hunting Grounds",
    townmap_x   = 150,
    townmap_y   = 220
})
log.info("new_arroyo_map: " .. tostring(new_map_id))

--
-- events.on('map_enter', function(map_id)
--   if map_id ~= test_caves_id then return end
--
--   if map.get_var(0) == 0 then
--     monitor.print("first time here")
--
--     map.set_var(0, 1)
--   else
--     monitor.print("already was here")
--   end
-- end)

events.on('map_enter', function(map_id)
  if map_id ~= 4 then return end

--   map.exit_grid.spawn_in_line(22748, 25156, {
--     map = new_map_id, tile = 21068, elevation = 0, rotation = 1, style = 0
-- }, 2)
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
