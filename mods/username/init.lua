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
