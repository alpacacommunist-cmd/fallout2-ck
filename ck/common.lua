local ffi = require('ffi')
local log = require('ck.system.log').new('ck/common.lua')

local common = {}

-- SECTION: object types
common.object_types = {
  [0] = "item",
  [1] = "critter",
  [2] = "scenery",
  [3] = "wall",
  [4] = "tile",
  [5] = "misc"
}

function common.object_type_by_pid(pid)
  return ffi.C.ck_ids_object_type_by_pid(pid)
end

function common.object_type_string(object_type)
  return common.object_types[object_type] or "unknown"
end

function common.object_type_string_by_pid(pid)
  local type_id = ffi.C.ck_ids_object_type_by_pid(pid)

  return common.object_types[type_id] or "unknown"
end

-- SECTION: null object mt
local null_object
local null_object_mt = {
  __index = function(self, key)
    if key == "id" or key == "pid" then
      return nil
    end

    return null_object
  end,

  __call = function(tbl, ...)
    return null_object
  end,

  __newindex = function(tbl, key, value)
  end,

  __tostring = function(self)
    return "[Null Object: object creation failed]"
  end
}

null_object = setmetatable({}, null_object_mt)
function common.null_object()
  return null_object
end

common._previous_mod_context = nil
common._current_mod_context = nil

-- current_mod_id (nullptr or string)
-- is set in mods.cc
function common.current_mod_id()
  return common._current_mod_context
end

function common.set_mod_context(mod_id)
  common._previous_mod_context = common._current_mod_context
  common._current_mod_context = mod_id

  ffi.C.ck_system_set_mod_context(mod_id)
end

function common.set_previous_mod_context()
  common.set_mod_context(common._previous_mod_context)
end

-- sets mod context and executes callbacks
function common.exec_with_mod_context(mod_id, callbacks)
  -- save previous context
  common.set_mod_context(mod_id)

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

  common.set_previous_mod_context()

  return all_success
end

-- mods.game_time_extender.init -> ../mods/game_time_extender/init.lua
function common.key_to_path(key)
  return "../" .. key:gsub("%.", "/") .. ".lua"
end

function common.mod_directory(mod_id)
  return string.format("../mods/%s/", mod_id)
end


return common
