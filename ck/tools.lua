local ffi = require('ffi')
local log = require('ck.system.log').new('ck/tools.lua')

local tools = {}

-- SECTION: object types
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
function tools.null_object()
  return null_object
end

tools._previous_mod_context = nil
tools._current_mod_context = nil

-- current_mod_id (nullptr or string)
-- is set in dispatcher.cc
function tools.current_mod_id()
  return tools._current_mod_context
  -- local mod_id = ffi.C.ck_system_current_mod_id()
  -- if mod_id == nil then return nil end
  --
  -- return ffi.string(mod_id)
end

function tools.set_mod_context(mod_id)
  tools._previous_mod_context = tools._current_mod_context
  tools._current_mod_context = mod_id

  ffi.C.ck_system_set_mod_context(mod_id)
end

function tools.set_previous_mod_context()
  tools.set_mod_context(tools._previous_mod_context)
end

-- sets mod context and executes callbacks
function tools.exec_with_mod_context(mod_id, callbacks)
  -- save previous context
  tools.set_mod_context(mod_id)

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

  tools.set_previous_mod_context()

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
