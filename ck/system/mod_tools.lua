local ffi = require("ffi")
local log = ck.log.new('mods.lua')

local mod_tools = {}

local function error_handler(mod_id, error)
  return debug.traceback(error, 2)
end

function mod_tools.exec_with_mod_context(mod_id, callback)
  mod_loading = mod_loading or false
  local raw_context = ffi.C.ck_get_current_mod_id()

  -- previous context (could be nullptr)
  local previous_mod_context = nil
  if raw_context ~= nil then
    previous_mod_context = ffi.string(raw_context)
  end

  ffi.C.ck_set_current_mod_context(mod_id)
  local success, result = xpcall(callback, error_handler)

  if not success then
    log.error("Error in mod [%s]!\nError: %s", mod_id, result)
  end

  -- restore context (could be nullptr)
  ffi.C.ck_set_current_mod_context(previous_mod_context)

  return success
end

return mod_tools
