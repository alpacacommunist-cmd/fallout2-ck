local ffi = require('ffi')

local ck = {}
--
-- GOTO sections: TOOLS, REGISTRIES, PUBLIC
--

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

local log = ck.log.new('ck/init.lua')

-- TOOLS
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

-- REGISTRIES
ck.registries = {
  --🎮📦
  -- Common
  default_mod_type = "gameplay",
  mod_types = { "gameplay", "library" },
  mod_load_sequence_by_type = { "library", "gameplay" },

  -- state.lua
  -- state.db.maps[map_id][mod_id] structure (keys flat list)
  state_mod_namespace_keys = { "objects", "timers" },
  -- quick ref table
  state_mod_namespace_keys_lookup = {},

  -- Mod specific
  -- ck/system/events.lua
  events  = {},

  -- ck/fallout2/objects/init.lua
  objects = {},

  -- ck/fallout2/dialogs.lua 💬
  dialogs = {},

  -- ck/fallout2/timers.lua ⏱️
  timers = {},
  timer_active_tags = {},
  timer_categories = { live = {}, evented = {} },

  -- ck/fallout2/critters.lua 🦂
  critter_spawn_counters = {},
  critter_active_tags = {},
  critter_respawns = {}
}

for _, key in ipairs(ck.registries.state_mod_namespace_keys) do
  ck.registries.state_mod_namespace_keys_lookup[key] = true
end

local available_listeners = { 'onGameStart', 'onEngineReady', 'onModReload',
  'onDayPassed', 'onHourPassed', 'time_advance',
  'onBeforeGameLoad', 'onGameLoaded',
  'onDialogStart', 'skill_used', 'critter_killed',
  'map_enter', 'map_update'
}

function ck.registries.init_mod(mod_id)
  table.insert(ck.active_mods, mod_id)

  ck.registries.events[mod_id] = {}
  for index, listener in ipairs(available_listeners) do
    ck.registries.events[mod_id][listener] = {}
  end

  ck.registries.objects[mod_id] = {}

  -- 💬
  ck.registries.dialogs[mod_id] = {}

  -- ⏱️
  ck.registries.timers[mod_id]  = {}
  ck.registries.timer_active_tags[mod_id] = {}
  ck.registries.timer_categories.live[mod_id] = {}
  ck.registries.timer_categories.evented[mod_id] = {}

  -- 🦂
  ck.registries.critter_spawn_counters[mod_id] = 0
  ck.registries.critter_respawns[mod_id] = {}
  ck.registries.critter_active_tags[mod_id] = {}
end

function ck.registries.clear_mod(mod_id)
  for index = #ck.active_mods, 1, -1 do
    if ck.active_mods[index] == mod_id then
      table.remove(ck.active_mods, index)
      break
    end
  end

  ck.registries.events[mod_id] = nil

  ck.registries.objects[mod_id] = nil

  -- 💬
  ck.registries.dialogs[mod_id] = nil

  -- ⏱️
  ck.registries.timers[mod_id]  = nil
  ck.registries.timer_active_tags[mod_id] = nil
  ck.registries.timer_categories.live[mod_id] = nil
  ck.registries.timer_categories.evented[mod_id] = nil

  -- 🦂
  ck.registries.critter_spawn_counters[mod_id] = nil
  ck.registries.critter_respawns[mod_id] = nil
  ck.registries.critter_active_tags[mod_id] = nil
end

function ck.registries.reset_map_context()
  for _, mod_id in ipairs(ck.active_mods) do
    ck.registries.objects[mod_id] = {}

    -- 💬
    ck.registries.dialogs[mod_id] = {}

    -- ⏱️
    ck.registries.timers[mod_id]  = {}
    ck.registries.timer_active_tags[mod_id] = {}
    ck.registries.timer_categories.live[mod_id] = {}
    ck.registries.timer_categories.evented[mod_id] = {}

    -- 🦂
    ck.registries.critter_spawn_counters[mod_id] = 0
    ck.registries.critter_respawns[mod_id] = {}
    ck.registries.critter_active_tags[mod_id] = {}
  end
end

-- PUBLIC
-- Mod gets this when requiring('ck')
ck.public = {
  language = ck.language,
  log = ck.log
}

return ck
