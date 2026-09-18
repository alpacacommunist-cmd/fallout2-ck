local ffi = require('ffi')

local ck = {}

ck = {
  -- quick ref data table
  active_mods = {},
  -- flat list of loaded mods
  active_mods_list = {},
  -- mods sorted by type
  active_mods_by_type = {},
  -- list of loaded lib mods
  loaded_libs = {},

  -- default language (is set to language from fallout2.cfg on init)
  language = 'english',

  -- common
  log = require('ck.system.log'),
  tools = {}
}

local log = ck.log.new('mods.lua')

-- current_mod_id (nullptr or string)
-- is set in dispatcher.cc
function ck.tools.current_mod_id()
  local mod_id = ffi.C.ck_get_current_mod_id()

  if mod_id == nil then return nil end

  return ffi.string(mod_id)
end

-- sets mod context and executes callbacks
function ck.tools.exec_with_mod_context(mod_id, callbacks)
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
function ck.tools.key_to_path(key)
  return "../" .. key:gsub("%.", "/") .. ".lua"
end


-- Mod gets this when requiring('ck')
ck.public = {
  language = ck.language,
  log = ck.log
}

return ck
