-- mods/arroyo_expanded/init.lua
print("[Mod] Loading Arroyo Expanded Mod...")

local events      = require('ck.fallout2.events')
local gameTime    = require('ck.fallout2.game_time')
local log         = require('ck.fallout2.log')
local map         = require('ck.fallout2.map')
local i18n        = require('ck.fallout2.i18n')

events.on('onGameLoaded', function()
end)

events.on('onMapEnter', function()
  local mapId = map.getId()

  log.print("Map id: " .. tostring(mapId))
  log.print("Entered map!")

  if (mapId == 4) then
    map.spawnCritter(16777218, 19908, 2000)
    -- map.spawnCritter(16777219, 19908, 2000)
  end
end)
