-- mods/username/test.lua
-- temp launch point

-- load system modules and expose them to engine
require('fallout2.events')
require('fallout2.config')

-- this will load registered mods
require('fallout2.loader')



-- imitate mods load, goes to engine eventually
ckInitializeMods()


local events = require('fallout2.events')
local gameTime = require('fallout2.game_time')
local log = require('fallout2.log')

events.on('onDayPassed', function()
  local date = gameTime.getDate()

  log.print(
    string.format(
      "Date: %d/%d/%d Hour: %d",
      date.day, date.month, date.year, date.hour
    )
  )

  log.print(
    "Total days: " .. gameTime.getTotalDays()
  )

  local isNight
  if gameTime.isNight() then
    isNight = true
  else
    isNight = false
  end

  log.print(isNight)
end)
