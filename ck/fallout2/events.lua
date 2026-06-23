-- ck/fallout2/events.lua
local unpack = table.unpack or unpack

local objects = require('ck.fallout2.objects')
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

  current_loading_mod = nil
}

-- Public mod API
-- Allows mod events subscription (fallout2.events.on('onGameStart'))
function events.on(event_name, callback)
  if not events.listeners[event_name] then
    log.warn("Attempted to subscribe to unknown event: " .. tostring(event_name))
    return
  end

  table.insert(events.listeners[event_name], {
    mod = events.current_loading_mod or "unknown",
    fn  = callback
  })
end

function events.emit(event_name, ...)
  local entries = events.listeners[event_name]

  if not entries then return end

  local args = { ... }

  for index, entry in ipairs(entries) do
    local ok, err = xpcall(function()
      entry.fn(unpack(args))
    end, debug.traceback)

    if not ok then
      log.error(string.format("in mod '%s' on event '%s' (#%d):\n%s", entry.mod, event_name, index, err))
    end
  end
end

function events.clearForMod(mod_name)
  for event_name, entries in pairs(events.listeners) do
    local clean_list = {}

    for _, entry in ipairs(entries) do
      if entry.mod ~= mod_name then
        table.insert(clean_list, entry)
      end
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
  log.info("Engine signaled: Game Loaded!")
  events.emit('onBeforeGameLoad')
end

function ckOnGameLoaded()
  log.info("Engine signaled: Game Loaded!")
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
  log.info("Engine signaled: Map Enter!")
  events.emit('onMapEnter')
end

function ckOnTimeAdvance(hours, minutes)
  log.info("Time Advanced on " .. tostring(hours) .. " h. and " .. tostring(minutes) .. " minutes")
  events.emit('onTimeAdvance', hours, minutes)
end

local UPDATE_INTERVAL  = 10
local last_update_time = 0

function ckOnMapUpdate(ticks)
  if (ticks - last_update_time) < UPDATE_INTERVAL then return end
  last_update_time = ticks

  for _, object in pairs(objects.registry) do
    if object._handle_map_update then
      local success, err = pcall(object._handle_map_update, object, ticks)

      if not success then
        log.error("in 'map_update' for object " .. tostring(object.id) .. ": " .. tostring(err))
      end
    end
  end
end

function ckOnProc(lua_id, proc_id)
  local object = objects.registry[lua_id]

  if not object then return false end
  return object:_handle_proc(proc_id)
end

function ckOnObjectsDestroyed()
  objects.registry = {}
  log.info("[CK Objects] Registry cleared")
end

return events
