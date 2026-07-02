-- mods/arroyo_expanded/init.lua
local monitor     = require('ck.fallout2.monitor')
local map         = require('ck.fallout2.map')
local dialogue    = require('ck.fallout2.dialogue')
local critters    = require('ck.fallout2.objects.critters')
local state       = require('ck.fallout2.state')
local player      = require('ck.fallout2.player')
local behaviors   = require('ck.fallout2.objects.critters.behaviors')
local items       = require('ck.fallout2.objects.items')
local quests      = require('ck.fallout2.quests')

events.on('onGameLoaded', function()
  log.info(player.stats.strength)
  log.info(player.stats.intelligence)
  log.info(player.gender)
  log.info(player.level)
end)

events.on('onModReload', function()
  log.warn('reloaded!')
end)

events.on('onMapEnter', function()
  local map_id = map.get_id()

  monitor.print("Map id: " .. tostring(map_id))
  monitor.print("Entered map!")

  if map_id ~= 4 then return end

  local alice = critters.register("alice_arroyo", 16777218, 19908, {
    base_stats = { strength = 8, endurance = 8, agility = 8, perception = 7 },
    name        = i18n.t('arroyo_expanded', 'alice_name'),
    description = i18n.t('arroyo_expanded', 'alice_description')
  })

  log.info("Alice endurance: " .. tostring(alice.base_stats.endurance))
  log.info("Alice hp: " .. tostring(alice:hp()))
  log.info("Alice max hp: " .. tostring(alice:max_hp()))
  log.info("Alice crit_chance: " .. tostring(alice.stats.critical_chance))

  alice
    :on('look_at', function(self) monitor.print(self.name) end)
    :on('description', function(self) monitor.print(self.description) end)
    :on('map_update', function(self) self:float_message('Здарова', 2) end)

  state.track(alice, { save_interval_seconds = 5 })

  alice:set_behavior(behaviors.patrol, { 16912, 17724, 18706, 20924, 21516 }, 5)

  alice:give_item(items.PID_KNIFE, 1)
  alice:give_item(items.PID_STIMPAK, 5)

  alice:on('dialogue_finished', function(self)
    log.debug("Dialogue finished with NPC ID: " .. tostring(self.id))

    if quests.get("erlang_refactoring") == quests.status.NOT_STARTED then
      quests.set("erlang_refactoring", quests.status.ACTIVE)

      log.info("Accepted quest!")
    end
  end)

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


  local alice_dialogue = require('.dialogs').alice_nodes
  dialogue.register(alice.id, alice_dialogue)
end)
