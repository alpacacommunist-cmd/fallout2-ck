local ffi = require('ffi')

--
-- GOTO sections: COMMON, REGISTRIES, PUBLIC
--
--
local ck = {
  -- quick ref data table
  active_mods = {},
  -- flat list of loaded mods
  active_mods_list = {},
  -- mods sorted by type
  active_mods_by_type = {},
  -- lua preload key for mod libs
  libs_preload_key = 'ck.libs.',
  -- list of loaded lib mods
  loaded_libs = {},

  -- default language (is set to language from fallout2.cfg on init)
  language = 'english',

  -- common
  log = require('ck.system.log'),
  common = {}
}

local log = ck.log.new('ck/init.lua')

-- #region COMMON {{{
local common = ck.common

common.object_types = {
  [0] = "item",
  [1] = "critter",
  [2] = "scenery",
  [3] = "wall",
  [4] = "tile",
  [5] = "misc",
  [6] = 'interface',
  [7] = 'inventory',
  [8] = 'head',
  [9] = 'background'
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

-- script_procs id -> name table
common.script_proc_names = {
  [0] = "no_proc",
  [1] = "start",
  [2] = "spatial",
  [3] = "description",
  [4] = "pickup",
  [5] = "drop",
  [6] = "use",
  [7] = "use_obj_on",
  [8] = "use_skill_on",
  [9] = "use_ad_on", [10] = "use_disad_on", -- ?
  [11] = "talk",
  [12] = "critter",
  [13] = "combat",
  [14] = "damage",
  [15] = "map_enter",
  [16] = "map_exit",
  [17] = "create",
  [18] = "destroy",
  [19] = "barter_init",
  [20] = "barter",
  [21] = "look_at",
  [22] = "timed",
  [23] = "map_update",
  [24] = "push",
  [25] = "is_dropping",
  [26] = "combat_is_starting",
  [27] = "combat_is_over",
}

-- script_procs name -> id table
common.script_procs = {}
for id, name in pairs(common.script_proc_names) do
  common.script_procs[name] = id
end

-- checks whether int/string id is valid (returns int value)
common.script_proc_id_normalized = function(proc_id)
  if type(proc_id) == "number" then
    if (ck.common.script_proc_names[proc_id] ~= nil) then
      return proc_id
    end
  end

  if (type(proc_id) == "string") then
    local proc = ck.common.script_procs[proc_id]
    if proc then return proc end
  end

  return nil
end

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

function common.mod_preload_key(mod_id)
  return ck.libs_preload_key .. mod_id
end
-- #endregion }}}

-- #region REGISTRIES {{{
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

  -- ck/system/events.lua 📞
  events = {},

  -- ck/fallout2/assets.lua 🌲
  assets = {},

  -- ck/fallout2/objects/init.lua
  objects = {},

  -- ck/fallout2/dialogs.lua 💬
  dialogs = {},

  -- ck/fallout2/proto/init.lua 🧬
  prototypes = {},
  prototype_tags = {},
  prototype_migrations = {},

  -- ck/fallout2/timers.lua ⏱️
  timers = {},
  timer_categories = { live = {}, evented = {} },

  -- 🧬protototypes (placeholder) (not used)

  -- ck/fallout2/critters.lua 🦂
  critter_spawn_counters = {},
  critter_active_tags = {},
  critter_respawns = {}
}

-- populate quick ref (lookup) tables
for _, key in ipairs(ck.registries.state_mod_namespace_keys) do
  ck.registries.state_mod_namespace_keys_lookup[key] = true
end

function ck.registries.init_mod(mod_id)
  -- 📞
  ck.registries.events[mod_id] = {}
  -- 🌲
  ck.registries.assets[mod_id] = {}
  ck.registries.objects[mod_id] = {}
  -- ck/fallout2/dialogs.lua 💬
  ck.registries.dialogs[mod_id] = {}
  -- ck/fallout2/proto/init.lua 🧬
  ck.registries.prototypes[mod_id] = { items = {}, critters = {} }
  ck.registries.prototype_tags[mod_id] = { items = {}, critters = {} }
  ck.registries.prototype_migrations[mod_id] = {}
  -- ck/fallout2/timers.lua ⏱️
  ck.registries.timers[mod_id]  = {}
  ck.registries.timer_categories.live[mod_id] = {}
  ck.registries.timer_categories.evented[mod_id] = {}
  -- ck/fallout2/critters.lua 🦂
  ck.registries.critter_spawn_counters[mod_id] = 0
  ck.registries.critter_respawns[mod_id] = {}
  ck.registries.critter_active_tags[mod_id] = {}
end

function ck.registries.clear_mod(mod_id)
  -- 📞
  ck.registries.events[mod_id] = nil
  -- 🌲
  ck.registries.assets[mod_id] = nil
  ck.registries.objects[mod_id] = nil
  -- 💬
  ck.registries.dialogs[mod_id] = nil
  -- 🧬
  ck.registries.prototypes[mod_id] = nil
  ck.registries.prototype_tags[mod_id] = nil
  ck.registries.prototype_migrations[mod_id] = nil
  -- ⏱️
  ck.registries.timers[mod_id]  = nil
  ck.registries.timer_categories.live[mod_id] = nil
  ck.registries.timer_categories.evented[mod_id] = nil
  -- 🦂
  ck.registries.critter_spawn_counters[mod_id] = nil
  ck.registries.critter_respawns[mod_id] = nil
  ck.registries.critter_active_tags[mod_id] = nil
end

function ck.registries.reset_map_context()
  for _, mod_id in ipairs(ck.active_mods_list) do
    ck.registries.objects[mod_id] = {}
    -- 💬
    ck.registries.dialogs[mod_id] = {}
    -- ⏱️
    ck.registries.timers[mod_id] = {}
    ck.registries.timer_categories.live[mod_id] = {}
    ck.registries.timer_categories.evented[mod_id] = {}
    -- 🧬
    ck.registries.prototypes[mod_id].critters = {}
    ck.registries.prototype_tags[mod_id].critters = {}
    -- 🦂
    ck.registries.critter_spawn_counters[mod_id] = 0
    ck.registries.critter_respawns[mod_id] = {}
    ck.registries.critter_active_tags[mod_id] = {}
  end
end
-- #endregion }}}

-- #region PUBLIC {{{
-- Mod gets this when requiring('ck')
ck.public = {
  language = ck.language,
  log = ck.log
}
-- #endregion }}}

return ck
