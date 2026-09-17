local ffi = require("ffi")
local log = require('ck.system.log').new('mods.lua')

local mod_tools = {}

function mod_tools.current_mod_id()
  local mod_id = ffi.C.ck_get_current_mod_id()

  if mod_id == nil then return nil end

  return ffi.string(mod_id)
end

function mod_tools.exec_with_mod_context(mod_id, callbacks)
  ffi.C.ck_set_current_mod_context(mod_id)

  if callbacks == nil then
    callbacks = {}
  elseif type(callbacks) == "function" then
    callbacks = { callbacks }
  end

  local all_success = true

  for _, callback in ipairs(callbacks) do
    local success, result = xpcall(callback, debug.traceback)

    if not success then
      log.error("Error in mod [%s]!\nError: %s", mod_id, result)
      all_success = false
      break
    end
  end

  ffi.C.ck_set_current_mod_context(nil)

  return all_success
end

-- mods.game_time_extender.init -> ../mods/game_time_extender/init.lua
function mod_tools.key_to_path(key)
  return "../" .. key:gsub("%.", "/") .. ".lua"
end

return mod_tools
