local ffi = require('ffi')

local tools = {}

tools.object_types = {
  [0] = "item",
  [1] = "critter",
  [2] = "scenery",
  [3] = "wall",
  [4] = "tile",
  [5] = "misc"
}

function tools.object_type_by_pid(pid)
  return ffi.C.ck_ids_object_type_by_pid(pid)
end

function tools.object_type_string(object_type)
  return tools.object_types[object_type] or "unknown"
end

function tools.object_type_string_by_pid(pid)
  local type_id = ffi.C.ck_ids_object_type_by_pid(pid)

  return tools.object_types[type_id] or "unknown"
end

-- current_mod_id (nullptr or string)
-- is set in dispatcher.cc
function tools.current_mod_id()
  local mod_id = ffi.C.ck_system_current_mod_id()
  if mod_id == nil then return nil end

  return ffi.string(mod_id)
end

-- sets mod context and executes callbacks
function tools.exec_with_mod_context(mod_id, callbacks)
  ffi.C.ck_system_set_mod_context(mod_id)

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

  ffi.C.ck_system_set_mod_context(nil)

  return all_success
end

-- mods.game_time_extender.init -> ../mods/game_time_extender/init.lua
function tools.key_to_path(key)
  return "../" .. key:gsub("%.", "/") .. ".lua"
end

function tools.mod_directory(mod_id)
  return string.format("../mods/%s/", mod_id)
end


return tools
