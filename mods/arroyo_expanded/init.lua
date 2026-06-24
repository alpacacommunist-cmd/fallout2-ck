-- mods/arroyo_expanded/init.lua
log.info("Loading Arroyo Expanded Mod...")

local monitor     = require('ck.fallout2.monitor')
local map         = require('ck.fallout2.map')
local dialogue    = require('ck.fallout2.dialogue')
local critters    = require('ck.fallout2.objects.critters')
local player      = require('ck.fallout2.player')
local behaviors   = require('ck.fallout2.behaviors')
local items       = require('ck.fallout2.objects.items')

local log   = ck.log.new('Arroyo Expanded')

events.on('onGameLoaded', function()
end)

events.on('onMapEnter', function()
  local map_id = map.get_id()

  monitor.print("Map id: " .. tostring(map_id))
  monitor.print("Entered map!")

  if map_id ~= 4 then return end

  local alice = critters.register("alice_arroyo", 16777218, 19908, {
    name        = i18n.t('arroyo_expanded', 'alice_name'),
    description = i18n.t('arroyo_expanded', 'alice_description')
  })

  alice
    :on('look_at', function(self) monitor.print(self.name) end)
    :on('description', function(self) monitor.print(self.description) end)
    :on('map_update', function(self) self:float_message('Здарова', 2) end)

  log.info("NPC ID: " .. tostring(alice.id))
  log.info("Alice tile: " .. alice:tile())
  log.info("Alice sid: " .. alice.sid)
  log.info("Alice tag: " .. alice.tag)

  alice:give_item(items.PID_KNIFE, 1)
  alice:give_item(items.PID_STIMPAK, 5)

  alice:animate()
    :walk_to(20913)
    :walk_to(21116)
    :play(16)
  :submit()

  local villager1 = critters.create(16777219, 21119, { name = "Житель Арройо" })
  local villager2 = critters.create(16777220, 21716, { name = "Охотник" })

  villager1:on('talk', function(self) self:float_message('Че хочешь?', 1) end)
    :set_behavior(behaviors.wander, 3)
  villager2:on('talk', function(self) self:float_message('Здарова, заебал', 4) end)
    :set_behavior(behaviors.wander, 2)

  state.track(alice, { save_interval_seconds = 5 })
  alice:set_behavior(behaviors.patrol, { 16912, 17724, 18706, 20924, 21516 }, 5)

  local alice_dialogue = require('.dialogs').alice_nodes
  dialogue.register(alice.id, alice_dialogue)
end)
