local ck = require('ck')
local timers = require('ck.fallout2.timers')
local validations = require('ck.fallout2.objects.critters.validations')

local log = ck.log.new('critters/respawns.lua')

local respawns = {}

-- Mod registered respawn timers:
-- { `mod_id` = { `respawn_tag` = { timer_tag = `timer_tag`, spawned_count = 0, ... }, ... } }
respawns.registry = ck.registries.critter_respawns

function respawns.generate_unique_tag(mod_id, tag)
  local respawn = respawns.registry[mod_id][tag]

  if not respawn then
    log.error("Respawn [%s] not for mod_id: [%s]", tag, mod_id)
    return nil
  end

  -- radscorpions_respawn_0, radscorpions_respawn_1 ...
  return tag .. "_respawn_" .. tostring(respawn.spawned_count)
end

-- register function
function respawns.register(tag, ticks, config)
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

  local timer = timers.register(timer_tag, timers.timer_types.periodic, ticks, nil, config.events_list)

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

function respawns.remove(mod_id, tag)
  local respawn = respawns.regitry[mod_id][tag]
  if not respawn then return false end

  if timers.remove(respawn.timer_tag) then
    log.debug("removed timer from mod's [%s] timers.registry: [%s]", mod_id, respawn.timer_tag)
    return true
  end

  respawns.registry[mod_id][tag] = nil
  log.debug("removed respawn queue from mod's [%s] respawns.registry: [%s]", mod_id, respawn.timer_tag)

  return false
end

function respawns.append(mod_id, pid, tile, config, spawn_function)
  local respawn = respawns.registry[mod_id][config.respawn]

  if not respawn then
    log.error("respawn [%s] not found! Skipping critter creation", config.respawn)
    return nil
  end

  local timer = timers.registry[mod_id][respawn.timer_tag]

  if not timer then
    log.error("timer [%s] not found! Skipping critter creation", respawn.timer_tag)
    return nil
  end

  -- config is passed to critter class (for critter_killed event)
  -- _respawn_queue attribute is used in critters.register to autogenerate tag
  config._respawn_queue = config.respawn
  -- nullify respawn (initially set by mod)
  config.respawn = nil
  -- prepare spawn callback
  callback = function() spawn_function(nil, pid, tile, config) end

  table.insert(timer.callbacks, callback)
  log.debug("added critter pid [%d], tile [%d] to respawn queue: [%s]", pid, tile, config._respawn_queue)

  return true
end

-- function respawns.append(mod_id, tag, callback)
--   local respawn = respawns.registry[mod_id][tag]
--
--   if not respawn then
--     logger.error("failed to find respawn queue [%s] for mod_id [%s]", tag, mod_id)
--     return false
--   end
--
--   local timer = timers.registry[mod_id][respawn.timer_tag]
--   if not timer then
--     logger.error("failed to find respawn timer [%s] for mod_id [%s]", respawn.timer_tag, mod_id)
--     return false
--   end
--
--   table.insert(timer.callbacks, callback)
--   return true
-- end

return respawns
