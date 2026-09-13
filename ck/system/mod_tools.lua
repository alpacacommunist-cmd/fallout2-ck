local ffi = require("ffi")
local log = ck.log.new('mods.lua')

local mod_tools = {}

local function error_handler(mod_id, error)
  return debug.traceback(error, 2)
end

function mod_tools.exec_with_mod_context(mod_id, callback)
  ffi.C.ck_set_current_mod_context(mod_id)
  local success, result = xpcall(callback, error_handler)

  if not success then
    log.error("Error in mod [%s]!\nError: %s", mod_id, result)
  end
  ffi.C.ck_set_current_mod_context(nil)

  return success
end

return mod_tools
