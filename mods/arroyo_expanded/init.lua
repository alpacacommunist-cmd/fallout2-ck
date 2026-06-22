-- mods/arroyo_expanded/init.lua
print("[Mod] Loading Arroyo Expanded Mod...")

local events      = require('ck.fallout2.events')
local log         = require('ck.fallout2.log')
local map         = require('ck.fallout2.map')
local dialogue    = require('ck.fallout2.dialogue')
local critters    = require('ck.fallout2.objects.critters')
local i18n        = require('ck.fallout2.i18n')
local player      = require('ck.fallout2.player')

events.on('onGameLoaded', function()
end)

events.on('onMapEnter', function()
  local map_id = map.get_id()

  log.print("Map id: " .. tostring(map_id))
  log.print("Entered map!")

  if map_id ~= 4 then return end

  local alice = critters.register(16777218, 19908, {
    name        = i18n.t('arroyo_expanded', 'alice_name'),
    description = i18n.t('arroyo_expanded', 'alice_description')
  })

  alice
    :on('look_at', function(self) log.print(self.name) end)
    :on('description', function(self) log.print(self.description) end)
    :on('map_update', function(self) self:float_message('hi', 1) end)

  print("[CK] NPC ID: " .. tostring(alice.id))

  alice:float_message("Здарова!", 2)

  local alice_dialogue = require('mods.arroyo_expanded.dialogs').alice_nodes
  dialogue.register(alice.id, alice_dialogue)
end)
