-- mods/arroyo_expanded/init.lua
local ffi = require('ffi')

local monitor     = require('ck.fallout2.monitor')
local map         = require('ck.fallout2.map')
local dialogue    = require('ck.fallout2.dialogue')
local critters    = require('ck.fallout2.objects.critters')
local state       = require('ck.fallout2.state')
local player      = require('ck.fallout2.player')
local behaviors   = require('ck.fallout2.objects.critters.behaviors')
local items       = require('ck.fallout2.objects.items')
local quests      = require('ck.fallout2.quests')
local skills      = require('ck.fallout2.objects.critters.skills')
local knowledge   = require('ck.fallout2.knowledge')
local proto       = require("ck.fallout2.proto")
local assets      = require("ck.fallout2.assets")
local sfx         = require('ck.fallout2.sound_sfx')

local golden_tail
events.on('onEngineReady', function()
  local PID_RADSCORPION_TAIL = 92
  inv_fid    = assets.resolve("arroyo_expanded:skilldex/scorpg.frm", 7)
  ground_fid = assets.resolve("arroyo_expanded:skilldex/scorpg_ground.frm", 0)

  golden_tail = proto.register_prototype(PID_RADSCORPION_TAIL, "arroyo_expanded:golden_scorpion_tail", {
    name        = "Золотой хвост скорпиона",
    description = "Мутировавший хвост редкого золотого радскорпиона. Выглядит очень дорого.",
    price  = 600,
    weight = 3,

    inv_fid    = inv_fid,
    ground_fid = ground_fid,

    -- usable = true
  })

  golden_tail:bind()
    :on('use', function()
      log.info(string.format("hello world [from golden scorpion tail PID: %s]", golden_tail.pid))
    end)
    :on('look_at', function()
      sfx.play("geiger")
    end)

  print("PID: " .. tostring(golden_tail.pid))
end)

events.on('onGameLoaded', function()
  log.info(player.stats.strength)
  log.info(player.stats.intelligence)
  log.info(player.gender)
  log.info(player.level)

  -- player.set_skill('melee_weapons', 10)
  player.add_perk_rank('toughness')
  log.info('small_guns: ' .. tostring(player.skills.small_guns))
  log.info('tougness: ' .. tostring(player.perks.toughness.rank))
end)

events.on('onModReload', function()
  log.warn('reloaded!')
end)

events.on('skill_used', function(skill, success_count, skill_bonus)
  local skill_name = skills.ID_MAP[skill] or "unknown_skill"

  log.info('skill: ' .. skill_name .. ' (ID: ' .. tostring(skill) .. ')')
  log.info('success: ' .. tostring(success_count))
  log.info('bonus: ' .. tostring(skill_bonus))
end)

local KN_SCORPION_HARVEST = knowledge.register({
  id       = 1,
  name     = "Scorpion Harvesting",
  art      = "arroyo_expanded:skilldex/scorpperk.frm",
  max_rank = 3,
  next_level = function(rank)
    return rank * 25
  end,
  ranks = {
    [1] = { desc = "You can cut off Radscorpion tails, but your cuts are messy and dangerous. Tails are heavy." },
    [2] = { desc = "Your cuts are clean. Tails now weigh 2 lbs instead of 4, and merchants value them more." },
    [3] = { desc = "Master Harvester. You extract pure venom glands alongside the tail, perfect for antidote crafting." }
  }
})

events.on('critter_killed', function(victim, killer)
  log.info("victim pid: " .. tostring(victim.pid))
  log.info("killer pid: " .. tostring(killer.pid))

  -- log.info("victim name: " .. victim:get_name())
  -- log.info("killer name: " .. killer:get_name())

  log.info("mod_id: " .. victim:get_mod_id())

  victim:give_item(golden_tail.pid, 1)
end)

events.on('onMapEnter', function(map_id)
  monitor.print("Map id: " .. tostring(map_id))
  monitor.print("Entered map!")

  if map_id ~= 4 then return end

  knowledge.grant(KN_SCORPION_HARVEST)

  local alice = critters.register("alice_arroyo", 16777218, 19908, {
    stats = { strength = 28, endurance = 18, agility = 8, perception = 7, luck = 25, hp = 75, max_hp = 90 },
    name        = i18n.t('arroyo_expanded', 'alice_name'),
    description = i18n.t('arroyo_expanded', 'alice_description')
  })

  alice.stats = { max_hp = 355, hp = 255 }

  log.info("Alice endurance: " .. tostring(alice.stats.endurance))
  log.info("Alice hp: " .. tostring(alice:hp()))
  log.info("Alice max hp: " .. tostring(alice:max_hp()))
  log.info("Alice crit_chance: " .. tostring(alice.stats.critical_chance))

  alice
    :on('map_update', function(self) self:float_message('Здарова', 2) end)

  state.track(alice, { save_interval_seconds = 5 })

  alice:set_behavior(behaviors.patrol, { 16912, 17724, 18706, 20924, 21516 }, 5)

  alice:give_item(items.PID_KNIFE, 1)
  alice:give_item(items.PID_STIMPAK, 5)

  alice:on('push', function(self)
    self:float_message('denied', 1)
    return true
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
    :set_behavior(behaviors.wander, 12)

  villager1:set_hp(1)

  local alice_dialogue = require('.dialogs').alice_nodes
  dialogue.register(alice.lua_id, alice_dialogue)

  alice:on('dialogue_finished', function(self)
    log.debug("Dialogue finished with NPC ID: " .. tostring(self.id))

    if quests.get("erlang_refactoring") == quests.status.NOT_STARTED then
      quests.set("erlang_refactoring", quests.status.ACTIVE)

      log.info("Accepted quest!")
    end
  end)
end)
