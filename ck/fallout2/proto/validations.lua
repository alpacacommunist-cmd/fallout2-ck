local utils = require('ck.system.utils')
local log   = require('ck.system.log').new('proto/validations.lua')

local proto_validations = {}

proto_validations.register_item_params = function(pid, lua_tag, config)
  local success = true

  if (type(pid) ~= "number" or pid <= 0) then
    log.error("source_pid must be specified")
    success = false
  end

  if (utils.is_blank(lua_tag)) then
    log.error("lua_tag must be specified")
    success = false
  end

  if (utils.is_blank(config.name)) then
    log.error("config.name must be specified")
    success = false
  end

  if (utils.is_blank(config.description)) then
    log.error("config.description must be specified")
    success = false
  end

  return success
end

return proto_validations
