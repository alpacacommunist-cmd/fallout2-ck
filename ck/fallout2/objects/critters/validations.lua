local utils = require('ck.system.utils')
local log   = require('ck.system.log').new('critters/validations.lua')

local critter_validations = {}

critter_validations.respawn_params = function(tag, ticks, config)
  local errors = false

  -- tag
  if (tag == nil or (utils.is_blank(tag))) then
    log.error("register_respawn_timer expects valid tag name")
    errors = true
  end

  -- ticks
  if (ticks == nil or (ticks <= 0)) then
    log.error("register_respawn_timer expects ticks > 0")
    errors = true
  end

  -- config
  config = config or {}
  config.events_list = config.events_list or { 'map_enter' }

  return tag, ticks, config, errors
end

return critter_validations
