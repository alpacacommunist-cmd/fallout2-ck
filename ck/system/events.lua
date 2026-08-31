-- ck/fallout2/events.lua
local unpack = table.unpack or unpack

local objects = require('ck.fallout2.objects')
local proto   = require('ck.fallout2.proto')
local state   = require('ck.fallout2.state')
local log     = ck.log.new('events.lua')

local object_ffi = require('ck.fallout2.classes.object_ffi')

local events = {
  -- listeners stack
  available_listeners = { 'onGameStart', 'onEngineReady', 'onModReload',
    'onDayPassed', 'onHourPassed', 'onTimeAdvance',
    'onBeforeGameLoad', 'onGameLoaded',
    'onDialogStart', 'skill_used', 'critter_killed',
    'map_enter', 'map_update'
  },

  listeners = {},
  map_update_interval  = 10,
  last_update_time     = 0
}

function events.init_mod(mod_id)
  events.listeners[mod_id] = {}
  for index, listener in ipairs(events.available_listeners) do
    events.listeners[mod_id][listener] = {}
  end
end

function events.register(mod_id, event_name, callback)
  if not events.listeners[mod_id] then events.init_mod(mod_id) end

  if not events.listeners[mod_id][event_name] then
    log.warn(string.format("[%s] Unknown event '%s'", mod_id, tostring(event_name)))
    return false
  end

  table.insert(events.listeners[mod_id][event_name], callback)
end

function events.emit(event_name, ...)
  for mod_id, _ in pairs(events.listeners) do events.emit_for_mod(mod_id, event_name, ...) end
end

function events.emit_for_mod(mod_id, event_name, ...)
  local mod_entries = events.listeners[mod_id]
  if not mod_entries then return end

  local callbacks = mod_entries[event_name]
  if not callbacks or #callbacks == 0 then return end

  local args = { ... }

  if event_name == "critter_killed" then
    -- args[1] - victim, args[2] - killer
    if args[1] then args[1] = object_ffi.from_ptr(args[1]) end
    if args[2] then args[2] = object_ffi.from_ptr(args[2]) end
  end

  for index, callback in ipairs(callbacks) do
    local ok, err = xpcall(function() callback(unpack(args)) end, debug.traceback)

    if not ok then
      log.error(string.format("Runtime error in mod '%s' on event '%s' (#%d):\n%s", mod_id, event_name, index, err))
    end
  end
end

-- global game events
-- e.g critter_killed, object_destroyed etc
function events.critter_killed(mod_id, event_name, victim, killer)
  victim = object_ffi.from_ptr(victim)
  killer = object_ffi.from_ptr(killer)

  events.emit_for_mod(mod_id, 'critter_killed', victim, killer)
end

function events.clear_for_mod(mod_id)
  events.listeners[mod_id] = nil
  log.info("Cleared listeners for mod: " .. mod_id)
end

function events.on_map_update(ticks)
  for _, object in pairs(objects.registry) do
    if object._handle_map_update then
      local success, err = pcall(object._handle_map_update, object, ticks)

      if not success then
        log.error("in 'map_update' for object " .. tostring(object.id) .. ": " .. tostring(err))
      end
    end
  end

  events.emit('map_update', ticks)
end

function events.on_proc(lua_id, proc_id, fixed_param)
  local object = objects.registry[lua_id]

  if not object then return false end
  return object:_handle_proc(proc_id, fixed_param)
end

function events.on_proto_proc(pid, proc_id, fixed_param)
  local proto = proto.registry[pid]

  if not proto then return false end
  return proto:_handle_proc(proc_id, fixed_param)
end

function events.clear_registries()
  -- Clears map context registries
  -- Clears registered dialogs
  local dialogue = require('ck.fallout2.dialogue')
  dialogue.clear_dialogs()

  -- Clears objects registry
  objects.clear_registry()
end

function events.map_context_change()
  -- Updates inventory/hp/tile etc
  state.sync_save()
  events.clear_registries()
end

-- Public mod API, mod gets sandboxed version from sandbox.lua
function events.on(event_name, callback)
end

return events
