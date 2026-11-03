local ck = require('ck')
local utils = require('ck.system.utils')

local log = ck.log.new('critters/validations.lua')

local critter_validations = {}

critter_validations.tag_presence_and_uniqueness = function(mod_id, tag)
  if utils.is_blank(tag) then
    log.error("tag is nil or blank")
    return false
  end

  if ck.registries.critter_active_tags[mod_id][tag] then
    log.error("tag is already registered: %s", tag)
    return false
  end

  return true
end

critter_validations.base_params = function(params)
  local success = true

  if type(params) ~= "table" then
    log.error "params must be a table"
    return false
  end

  -- pid
  if (type(params.pid) ~= "number" or params.pid <= 0) then
    log.error("pid(int) must be specified")
    success = false
  end

  -- tile
  if (type(params.tile) ~= "number" or params.tile <= 0) then
    log.error("tile(int) must be specified")
    success = false
  end

  return success
end

return critter_validations
