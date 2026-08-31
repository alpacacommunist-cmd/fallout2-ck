-- mods/arroyo_expanded/init.lua
local ffi = require('ffi')

local monitor     = require('ck.fallout2.monitor')
local map         = require('ck.fallout2.map')
local dialogue    = require('ck.fallout2.dialogue')
local critters    = require('ck.fallout2.objects.critters')
local player      = require('ck.fallout2.player')
local behaviors   = require('ck.fallout2.objects.critters.behaviors')
local items       = require('ck.fallout2.objects.items')
local quests      = require('ck.fallout2.quests')
local skills      = require('ck.fallout2.objects.critters.skills')
local knowledge   = require('ck.fallout2.knowledge')
local proto       = require("ck.fallout2.proto")
local assets      = require("ck.fallout2.assets")
local sfx         = require('ck.fallout2.sound_sfx')
local objects     = require('ck.fallout2.objects')

local PID_RADSCORPION_TAIL = 92
local inv_fid    = assets.resolve("arroyo_expanded:skilldex/scorpg.frm", 7)
local ground_fid = assets.resolve("arroyo_expanded:skilldex/scorpg_ground.frm", 0)

local golden_tail = proto.register_prototype(PID_RADSCORPION_TAIL, "arroyo_expanded:golden_scorpion_tail", {
  object_type = proto.types.item,

  name        = "Золотой хвост скорпиона",
  description = "Мутировавший хвост редкого золотого радскорпиона. Выглядит очень дорого.",
  price  = 600,
  weight = 3,

  inv_fid    = inv_fid,
  ground_fid = ground_fid,

  -- usable = true
})

print("PID: " .. tostring(golden_tail.pid))

golden_tail:bind()
:on('use', function()
  log.info("hello world [from golden scorpion tail PID: %s]", golden_tail.pid)
end)
:on('look_at', function()
  sfx.play("geiger")
end)

events.on('onGameLoaded', function()
  log.info(player.stats.strength)
  log.info(player.stats.intelligence)
  log.info(player.gender)
  log.info(player.level)

  -- player.set_skill('melee_weapons', 10)
  player.add_perk_rank('toughness')
  player.add_perk_rank('toughness')
  player.add_perk_rank('rad_resistance')
  log.info('small_guns: ' .. tostring(player.skills.small_guns))
  log.info('tougness: ' .. tostring(player.perks.toughness.rank))
end)

events.on('onModReload', function()
  log.warn('reloaded!')
end)

events.on('skill_used', function(skill, success_count, skill_bonus)
  local skill_name = skills.ID_MAP[skill] or "unknown_skill"

  log.info('skill: %s (ID: %d)', skill_name, skill)
  log.info('success: %d', success_count)
  log.info('bonus: %d', skill_bonus)
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

  victim:give_item(golden_tail.pid, 1, true)
end)

events.on('map_enter', function(map_id)
  -- player.add_experience(155000)
  monitor.print("Map id: " .. tostring(map_id))
  monitor.print("Entered map!")

  if map_id ~= 4 then return end

  knowledge.grant(KN_SCORPION_HARVEST)

  -- 16777351, 16777391, 16777349, 16777347, 16777345, 16777343, 16777341
  local alice = critters.register("alice_arroyo", 16777255, 19908, {
    stats = { strength = 28, endurance = 18, agility = 8, perception = 7, luck = 25, hp = 75, max_hp = 90 },
    skills = { small_guns = 200 },
    name        = i18n.t('arroyo_expanded', 'alice_name'),
    description = i18n.t('arroyo_expanded', 'alice_description'),
    ai_packet = 'Merc Captain',
    team = 0
  })
  --
  alice.stats = { max_hp = 355, hp = 255 }

  log.info("Alice endurance: %d", alice.stats.endurance)
  log.info("Alice hp: %d", alice:hp())
  log.info("Alice max hp: %d", alice:max_hp())
  log.info("Alice crit_chance: %d", alice.stats.critical_chance)
  log.info("Alice small_weapons: %d", alice.skills.small_guns)

  if (alice:has_inventory()) then
    log.info("Alice inventory is managed through state")
  else
    log.info("Adding new items to Alice's inventory")

    alice:give_item(items.PID_KNIFE, 1)
    alice:give_item(items.PID_STIMPAK, 5)

    alice:give_item(10, 1)
    alice:give_item(34, 10)
  end

  alice:take_out_weapon()

  alice:on('map_update', function(self) self:float_message('Здарова', 2) end)
  alice:set_behavior(behaviors.patrol, { 16912, 17724, 18706, 20924, 21516 }, 5)

  alice:on('push', function(self)
    self:float_message('denied', 1)
    return true
  end)

  alice:animate()
    :walk_to(20913)
    :walk_to(21116)
    :play(16)
    -- :play(38)
  :submit()

  local villager1 = critters.create(16777219, 21119, { team = 0 })
  local villager2 = critters.create(16777220, 21716, { team = 0 })

  log.info("villager1 gender: %d", villager1:gender())
  log.info("villager2 gender: %d", villager2:gender())

  villager1:on('talk', function(self) self:float_message('Че хочешь?', 1) end)
    :set_behavior(behaviors.wander, 3)
  villager2:on('talk', function(self) self:float_message('Здарова, заебал', 4) end)
    :set_behavior(behaviors.wander, 12)

  radscorpion1 = critters.create(16777221, 27916, { team = 2 })
  radscorpion2 = critters.create(16777221, 28513, { team = 2 })

  villager1:set_hp(1)
  --
  local alice_dialogue = require('.dialogs').alice_nodes
  dialogue.register(alice.lua_id, alice_dialogue)

  -- female_trapper_prototype = critters.allocate_prototype(16777351,
  --   { name = "Female Trapper", description = "F", ai_packet = 'Merc Captain' }
  -- )
  -- female_trapper_prototype
  --   :set_stats({ strength = 10, agility = 9, luck = 8})
  --   :set_skills({ unarmed = 100, small_guns = 85 })

  -- local trapper1 = critters.create(female_trapper_prototype.pid, 20909)
  -- log.info("trapper1 unarmed: %d", trapper1.skills.unarmed)

  alice:on('dialogue_finished', function(self)
    log.debug("Dialogue finished with NPC ID: " .. tostring(self.id))

    if quests.get("erlang_refactoring") == quests.status.NOT_STARTED then
      quests.set("erlang_refactoring", quests.status.ACTIVE)

      log.info("Accepted quest!")
    end
  end)
end)
