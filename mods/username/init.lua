-- mods/username/init.lua
print("[Mod] Loading Username Test Mod...")

local events = require('fallout2.events')
local gameTime = require('fallout2.game_time')
local log = require('fallout2.log')
local respawn = require('fallout2.respawn')
local map = require('fallout2.map')
local constants = require('fallout2.constants')
local huntingGrounds = require('username.hunting_grounds')

local huntingGroundsState = {
  lastRespawnDay = 0,
  respawnDays = 3
}

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

events.on('onGameLoaded', function()
  respawn.try(huntingGroundsState, function()
    log.print("The hunting grounds feel alive again.")
  end)
end)

local function logMapEnter(mapId)
  log.print("Map id: " .. tostring(mapId))
  log.print("Entered map!")
end

events.on('onMapEnter', function()
  local mapId = map.getId()

  logMapEnter(mapId)

  if not huntingGrounds.isMap(mapId) then
    return
  end

  huntingGrounds.spawnCreatures()
end)

