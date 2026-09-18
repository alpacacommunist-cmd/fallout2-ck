-- ck/fallout2/state.lua
local ck    = require('ck')
local ffi   = require('ffi')
local utils = require('ck.system.utils')

local registries = require('ck.system.registries')

local state = {}
local log   = ck.log.new('state.lua')

state.create_mod_table = function()
  local mod_namespace = {}

  setmetatable(mod_namespace, {
    __index = function(self, key)
      if registries.state_mod_namespace_keys_lookup[key] then
        local sub_table = {}
        rawset(self, key, sub_table)
        return sub_table
      end
    end
  })

  return mod_namespace
end

state.create_map_table = function()
  local map = {}
  setmetatable(map, {
    __index = function(self, mod_id)
      local sub_table = state.create_mod_table()
      rawset(self, mod_id, sub_table)
      return sub_table
    end,
    __newindex = function(self, mod_id, value)
      rawset(self, mod_id, value)
    end
  })
  return map
end

local db_init_state = {
  ["global"] = {},
  ["player"] = { knowledge = {} },
  ["proto_list"] = {},
  ["maps"]   = setmetatable({}, {
    __index = function(self, map_id)
      local map_table = state.create_map_table()
      rawset(self, map_id, map_table)
      return map_table
    end
  })
}

state.db = { player = db_init_state.player, global = db_init_state.global, maps = db_init_state.maps }

-- gets marshalld json -> lua from backend
function state.sync_load(loaded_db)
  state.db = loaded_db

  state.db.global = state.db.global or db_init_state.global
  state.db.player = state.db.player or db_init_state.player
  state.db.maps   = state.db.maps or db_init_state.maps
  state.db.proto_list = state.db.proto_list or db_init_state.proto_list

  -- utils.print_table(state.db.proto_list, log)
end


-- while running through entities in sync_save, remember active tags
-- used to GC old tags that were removed from mod code
local relevant_timers_tag_list  = {}

-- returns lua, backend marshalls it to json and saves
function state.sync_save()
  if ck.map_id == -1 then return state.db end

  log.header("mods list:")
  utils.print_table(ck.active_mods_list, log)

  local current_map = state.db.maps[ck.map_id] --rawget(state.db.maps, ck.map_id)

  if not current_map then
    log.header("Nothing to save on a map")
    return state.db
  end

  -- check active mods
  log.header("GC:")
  for _, mod_id in ipairs(ck.active_mods_list) do
    local mod_map_db = current_map[mod_id] -- rawget(current_map, mod_id)
    if (mod_map_db == nil) then
      log.debug("Mod [%s] has nothing to save on a map", mod_id)
      goto continue
    end

    -- timers
    -- `maps.id.mod_id.timers` e.g. maps.4.arroyo_expanded.timers
    local mod_timers = registries.timers[mod_id] or {}

    for tag, timer in pairs(mod_timers) do
      mod_map_db.timers[tag] = { created_at = timer.created_at, timer_type = timer.timer_type }

      relevant_timers_tag_list[tag] = true
    end

    -- objects
    -- `maps.id.mod_id.objects` e.g. maps.4.arroyo_expanded.objects
    local mod_objects = registries.objects[mod_id] or {}

    for _, object in ipairs(mod_objects) do
      if not object.lua_id or not object.mod_id or not object.tag or object.modified then
        goto continue
      end

      mod_map_db.objects[object.tag] = mod_map_db.objects[object.tag] or {}

      local object_state = mod_map_db.objects[object.tag]

      if object.tile then object_state.tile = object:tile() end
      if object.hp   then object_state.hp   = object:hp()   end

      object_state.id = object:id()
      object_state.inventory = object:inventory_table()

      ::continue::
    end

    -- [Garbage collection] ✨
    -- removes elements outside allowed scope
    for key in pairs(mod_map_db) do
      if not registries.state_mod_namespace_keys_lookup[key] then
        log.debug("GC: Removing non-whitelisted mod namespace element: [%s]", key)
        mod_map_db[key] = nil
      end
    end

    -- [Garbage Collection] ✨
    -- removes obsolete timers
    for tag in pairs(mod_map_db.timers) do
      if not relevant_timers_tag_list[tag] then
        log.debug("GC: Removing obsolete timer tag '%s' from mod '%s'", tag, mod_id)
        mod_map_db.timers[tag] = nil
      end
    end

    -- [Garbage Collection] ✨
    -- removes obsolete object tags
    for tag in pairs(mod_map_db.objects) do
      if not registries.critter_relevant_tags[mod_id][tag] then
        log.debug("GC: Removing obsolete critter tag '%s' from mod '%s'", tag, mod_id)
        mod_map_db.objects[tag] = nil
      end
    end

    -- [Garbage collection] ✨
    -- removes mod from map namespace if both timers and objects are empty
    if next(mod_map_db.objects) == nil and next(mod_map_db.timers) == nil then
      log.debug("GC: Removing maps[%d][%s] mod namespace", ck.map_id, mod_id)
      current_map[mod_id] = nil
    end

    ::continue::
  end

  -- Garbage Collection ✨
  -- removes map_id namespace if empty
  if next(current_map) == nil then
    log.debug("GC: Removing maps[%d]", ck.map_id)
    state.db.maps[ck.map_id] = nil
  end

  log.header("state_table:")
  utils.print_table(state.db, log)

  return state.db
end

function state.get_proto_list()
  return state.db.proto_list
end

-- Backend calls this to push registered prototypes
function state.receive_proto_list(data_address, size)
  local protos = ffi.cast("ItemProtoLuaView*", data_address)

  state.db.proto_list = {}

  for index = 0, size - 1 do
    local proto = protos[index]

    local pid     = proto.pid
    local lua_tag = ffi.string(proto.lua_tag)

    table.insert(state.db.proto_list, { id = pid, tag = lua_tag })

    log.debug(string.format("Index: %d, PID: %d, Tag: %s", index, pid, lua_tag))
  end
end

local function get_mod_storage(section, mod_id)
  if not mod_id then return nil end
  local map_id = ffi.C.ck_map_get_id()

  if section == "global" then
    state.db.global[mod_id] = state.db.global[mod_id] or {}

    return state.db.global[mod_id]
  elseif section == "maps" then
    if map_id == -1 then return nil end

    state.db.maps[map_id] = state.db.maps[map_id] or {}
    state.db.maps[map_id][mod_id] = state.db.maps[map_id][mod_id] or {}
    return state.db.maps[target_map][m_id]
  end
end

function state.set_local(key, value, mod_id)
  local storage = get_mod_storage("maps", mod_id)
  if storage then storage[key] = value end
end

function state.get_local(key, default, mod_id)
  local storage = get_mod_storage("maps", mod_id)
  if storage and storage[key] ~= nil then return storage[key] end
  return default
end

function state.set_global(mod_id, sub_section, key, value)
  state.db.global[mod_id] = state.db.global[mod_id] or {}
  state.db.global[mod_id][sub_section] = state.db.global[mod_id][sub_section] or {}
  state.db.global[mod_id][sub_section][key] = value
end

function state.get_global(mod_id, sub_section, key)
  if state.db.global[mod_id] and state.db.global[mod_id][sub_section] then
    return state.db.global[mod_id][sub_section][key]
  end
  return nil
end

function state.get_stored_object_data(mod_id, map_id, tag)
  if not (state.db.maps[map_id] and state.db.maps[map_id][mod_id]) then
    return nil
  end

  utils.print_table(state.db.maps[map_id][mod_id])
  return state.db.maps[map_id][mod_id]["objects"][tag]
end

function state.get_state_data(mod_id, map_id, tag)
  log.debug(string.format("mod_id: %s, map_id: %d, tag: %s", mod_id, map_id, tag))
  data = state.get_stored_object_data(mod_id, map_id, tag)

  if data then return data else return {} end
end

return state
