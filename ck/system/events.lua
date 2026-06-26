-- ck/fallout2/events.lua
local unpack = table.unpack or unpack

local ffi = require("ffi")

ffi.cdef[[
    void ck_registry_clear();
]]

local objects = require('ck.fallout2.objects')
local state   = require('ck.fallout2.state')
local log     = ck.log.new('CK Events')

local events = {
  -- listeners stack
  listeners = {
    onModReload = {},
    onGameStart = {},
    onDayPassed = {},
    onHourPassed = {},
    onBeforeGameLoad = {},
    onGameLoaded = {},
    onTimeAdvance = {},
    onDialogStart = {},
    onMapEnter = {}
  },

  current_active_mod = nil
}

local MAP_UPDATE_INTERVAL  = 10
local last_update_time = 0

-- Public mod API
-- Allows mod events subscription (fallout2.events.on('onGameStart'))
function events.on(event_name, callback)
  if not events.listeners[event_name] then
    log.warn("Attempted to subscribe to unknown event: " .. tostring(event_name))
    return
  end
end

function events.emit(event_name, ...)
  local entries = events.listeners[event_name]

  if not entries then return end

  local args = { ... }

  for index, entry in ipairs(entries) do
    events.current_active_mod = entry.mod

    local ok, err = xpcall(function() entry.fn(unpack(args)) end, debug.traceback)

    if not ok then
      log.error(string.format("in mod '%s' on event '%s' (#%d):\n%s", entry.mod, event_name, index, err))
    end
  end

  events.current_active_mod = nil
end

function events.clear_for_mod(mod_name)
  for event_name, entries in pairs(events.listeners) do
    local clean_list = {}

    for _, entry in ipairs(entries) do
      if entry.mod ~= mod_name then table.insert(clean_list, entry) end
    end

    events.listeners[event_name] = clean_list
  end
  log.info("Cleared listeners for mod: " .. mod_name)
end

-- ckHookOnGameStart (C) calls it when game started (interface initiated)
function ckOnGameStart()
  log.info("Engine signaled: Game Start! Firing listeners...")
  events.emit('onGameStart')
end

function ckOnBeforeGameLoad()
  events.emit('onBeforeGameLoad')
end

function ckOnGameLoaded()
  log.info("last_update_time: " .. tostring(last_update_time))
  last_update_time = 0

  events.emit('onGameLoaded')
end

function ckOnDayPassed()
  log.info("Engine signaled: Day Passed!")
  events.emit('onDayPassed')
end

function ckOnHourPassed()
  log.info("Engine signaled: Hour Passed!")
  events.emit('onHourPassed')
end

function ckOnMapEnter()
  state.clear_tracked_objects() -- clears state-tracker
  objects.clear_registry()      -- clears lua pointers
  ffi.C.ck_registry_clear()     -- reset C registry

  events.emit('onMapEnter')
end

function ckOnTimeAdvance(hours, minutes)
  log.info("Time Advanced on " .. tostring(hours) .. " h. and " .. tostring(minutes) .. " minutes")
  events.emit('onTimeAdvance', hours, minutes)
end


function ckOnMapUpdate(ticks)
  if (ticks - last_update_time) < MAP_UPDATE_INTERVAL then return end
  last_update_time = ticks

  for _, object in pairs(objects.registry) do
    if object._handle_map_update then
      local success, err = pcall(object._handle_map_update, object, ticks)

      if not success then
        log.error("in 'map_update' for object " .. tostring(object.id) .. ": " .. tostring(err))
      end
    end
  end

  state.update_tracked_objects(ticks)
end

function ckOnProc(lua_id, proc_id)
  local object = objects.registry[lua_id]

  if not object then return false end
  return object:_handle_proc(proc_id)
end

function ckOnObjectsDestroyed()
end

function events.emit_for_mod(mod_name, event_name, ...)
  local entries = events.listeners[event_name]
  if not entries then return end

  local args = { ... }

  for index, entry in ipairs(entries) do
    if entry.mod == mod_name then
      events.current_active_mod = mod_name

      local ok, err = xpcall(function() entry.fn(unpack(args)) end, debug.traceback)

      if not ok then
        log.error(string.format("in mod '%s' on localized event '%s' (#%d):\n%s", entry.mod, event_name, index, err))
      end
    end
  end

  events.current_active_mod = nil
end

return events
