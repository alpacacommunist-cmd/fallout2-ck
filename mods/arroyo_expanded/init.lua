-- mods/arroyo_expanded/init.lua
print("[Mod] Loading Arroyo Expanded Mod...")

local events      = require('ck.fallout2.events')
local gameTime    = require('ck.fallout2.game_time')
local log         = require('ck.fallout2.log')
local map         = require('ck.fallout2.map')
local i18n        = require('ck.fallout2.i18n')
local dialogue    = require('ck.fallout2.dialogue')
local critters    = require('ck.fallout2.objects.critters')

local dialogue = require('ck.fallout2.dialogue')

events.on('onGameLoaded', function()
end)

events.on('onMapEnter', function()
  local map_id = map.get_id()

  log.print("Map id: " .. tostring(map_id))
  log.print("Entered map!")

  if map_id ~= 4 then return end

  local dialogs = require('mods.arroyo_expanded.dialogs')

  local alice = critters.register(16777218, 19908, {
    name = "Alice",
    description = "Привет"
  })

  alice
    :on('look_at', function(self) log.print("You see Alice") end)
    :on('description', function(self) log.print(self.description) end)

  print("[CK] NPC ID: " .. tostring(alice.id))

  dialogue.register(alice.id, dialogs.alice_nodes.en)
end)
