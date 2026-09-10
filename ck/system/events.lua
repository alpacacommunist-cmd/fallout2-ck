-- ck/fallout2/events.lua
local unpack = table.unpack or unpack
local ffi = require("ffi")

local registries = require('ck.system.registries')

local objects = require('ck.fallout2.objects')
local proto   = require('ck.fallout2.proto')

local log     = ck.log.new('events.lua')
local utils   = require('ck.system.utils')

local object_ffi = require('ck.fallout2.classes.object_ffi')

local events = {}

events.listeners = registries.events

function events.register(mod_id, event_name, callback)
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

  for index, callback in ipairs(callbacks) do
    local ok, err = xpcall(function() callback(unpack(args)) end, debug.traceback)

    if not ok then
      log.error(string.format("Runtime error in mod '%s' on event '%s' (#%d):\n%s", mod_id, event_name, index, err))
    end
  end
end

-- global game events
-- e.g critter_killed, object_destroyed, skill_used etc
function events.critter_killed(victim, killer)
  victim = object_ffi.from_ptr(victim)
  killer = object_ffi.from_ptr(killer)

  for mod_name, mod_events in pairs(events.listeners) do
    local callbacks = mod_events.critter_killed

    if not callbacks then return end

    for index = 1, #callbacks do
      log.debug("global event 'critter_killed' for mod: %s", mod_name)

      local callback = callbacks[index]
      local success, result = xpcall(callback, function(err)
        return debug.traceback(string.format("[%s] Runtime Error: %s", mod_name, tostring(err)), 2)
      end, victim, killer)
    end
  end
end

function events.clear_for_mod(mod_id)
  events.listeners[mod_id] = {}
  log.info("Cleared listeners for mod: " .. mod_id)
end

function events.on_map_update(ticks)
  -- update timers (timed events)
  local timers = require('ck.fallout2.timers')
  timers.check_live_timers(ticks)

  -- handle map_update for lua objects
  for _, mod_id in ipairs(ck.active_mods) do
    for _, object in pairs(objects.registry[mod_id]) do
      if not object._handle_map_update then
        goto continue
      end

      local success, err = pcall(object._handle_map_update, object, ticks)
      if not success then
        log.error("in 'map_update' for object " .. tostring(object.lua_id) .. ": " .. tostring(err))
      end

      ::continue::
    end
  end

  events.emit('map_update', ticks)
end

function events.on_proc(lua_id, proc_id, fixed_param, mod_id)
  mod_id = ffi.string(mod_id)
  if objects.registry[mod_id] == nil then return false end

  local object = objects.registry[mod_id][lua_id]
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
  local dialogue = require('ck.fallout2.dialogue')
  local critters = require('ck.fallout2.objects.critters')
  local timers   = require('ck.fallout2.timers')

  -- Clears registered dialogs
  dialogue.clear_dialogs()
  -- Clears registered critter spawns
  critters.reset_spawn_counters()
  -- Clears objects registry
  objects.clear_registry()
  -- Clears timers registry
  timers.clear_registry()
end

-- is supposed to be called before map exit
-- to update inventory/hp/tile and timers in state db
function events.map_context_change()
  local state = require('ck.fallout2.state')
  state.sync_save()

  events.clear_registries()
end

-- Makes sure state db tables are initialized
-- Updates global map-related meta
function events.map_enter(map_id)
  events.clear_registries()

  ck.map_id = map_id

  local state = require('ck.fallout2.state')

  state.db.maps[map_id] = state.db.maps[map_id] or {}
  for _, mod_id in ipairs(ck.active_mods) do
    state.db.maps[map_id][mod_id] = state.db.maps[map_id][mod_id] or {}
    local mod_table = state.db.maps[map_id][mod_id]

    -- allowed tables
    mod_table.objects = mod_table.objects or {}
    mod_table.timers  = mod_table.timers or {}
  end

  -- utils.print_table(state.db, log)
end

-- Public mod API, mod gets sandboxed version from sandbox.lua
function events.on(event_name, callback)
end

return events
