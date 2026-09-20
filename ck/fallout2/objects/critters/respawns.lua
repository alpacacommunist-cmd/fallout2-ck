local ck = require('ck')
local timers = require('ck.fallout2.timers')
local validations = require('ck.fallout2.objects.critters.validations')

local log = ck.log.new('critters/respawns.lua')

local respawns = {}

-- Mod registered respawn timers:
-- { `mod_id` = { `respawn_tag` = { timer_tag = `timer_tag`, spawned_count = 0, ... }, ... } }
respawns.registry = ck.registries.critter_respawns

function respawns.autogenerate_unique_tag(mod_id, tag)
  local respawn = respawns.registry[mod_id][tag]

  if not respawn then
    log.error("Respawn [%s] not for mod_id: [%s]", tag, mod_id)
    return nil
  end

  -- radscorpions_respawn_0, radscorpions_respawn_1 ...
  return tag .. "_respawn_" .. tostring(respawn.spawned_count)
end

-- register function
function respawns.register_respawn(tag, ticks, config)
  tag, ticks, config, errors = validations.respawn_params(tag, ticks, config)
  if errors then return nil end

  -- tag is written to respawns.registry
  -- timer_tag is written to timers.registry
  local timer_tag = tag .. "_respawn_timer"

  -- check if timer already exists
  local mod_id = ck.tools.current_mod_id()

  if (respawns.registry[mod_id][tag]) then
    log.debug("Mod respawn [%s] already exists", tag)
    return nil
  end

  if (timers.registry[mod_id][timer_tag]) then
    log.debug("Mod timer [%s] already exists", tag)
  end

  local timer  = timers.register_timer(timer_tag, timers.timer_types.periodic, ticks, nil, config.events_list)

  if not timer then
    log.error("failed to create respawn_timer: [%s]", tag)
    return nil
  end

  local respawn = {
    registered_at = timers.current_ticks(),
    timer_tag     = timer_tag,
    spawned_count = 0
  }

  respawns.registry[mod_id][tag] = respawn
end

-- remove function
function respawns.remove_respawn_timer(tag)
  local mod_id = ck.tools.current_mod_id()
  local respawn = respawns.regitry[mod_id][tag]

  if not respawn then return false end

  if timers.remove(respawn.timer_tag) then
    log.debug("removed timer from mod's [%s] timers.registry: [%s]", mod_id, respawn.timer_tag)
    return true
  end

  return false
end

function respawns.add_callback(tag, callback)
end

return respawns
