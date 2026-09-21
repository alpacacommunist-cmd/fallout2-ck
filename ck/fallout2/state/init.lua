-- ck/fallout2/state.lua
local ck    = require('ck')
local ffi   = require('ffi')
local utils = require('ck.system.utils')
local state_gc = require('ck.fallout2.state.gc')

local state = {}
local log   = ck.log.new('state/init.lua')

local db_init_state = {
  ["global"] = {},
  ["player"] = { knowledge = {} },
  ["proto_list"] = {},

  ["maps"] = {}
}

state.db = {
  ["global"] = db_init_state.global,
  ["player"] = db_init_state.player,
  ["proto_list"] = db_init_state.proto_list,

  ["maps"] = db_init_state.maps
}

-- ensures mod's state table structure is present
function state.ensure_mod_namespace(mod_id)
  if ck.map_id == -1 then return nil end
  state.db.maps[ck.map_id] = state.db.maps[ck.map_id] or {}

  local current_map_db = state.db.maps[ck.map_id]
  current_map_db[mod_id] = current_map_db[mod_id] or {}

  for _, key in ipairs(ck.registries.state_mod_namespace_keys) do
    current_map_db[mod_id][key] = current_map_db[mod_id][key] or {}
  end

  return state.db.maps[ck.map_id][mod_id]
end

-- gets marshalld json -> lua from backend
function state.sync_load(loaded_db)
  state.db = loaded_db or {}

  state.db.global = state.db.global or db_init_state.global
  state.db.player = state.db.player or db_init_state.player
  state.db.proto_list = state.db.proto_list or db_init_state.proto_list
  state.db.maps = state.db.maps or db_init_state.maps
end

-- returns lua, backend marshalls it to json and saves
function state.sync_save()
  if ck.map_id == -1 then return state.db end

  -- print current mods header
  log.header("mods list:")
  utils.print_table(ck.active_mods_list, log)

  -- current_map state db
  local current_map = state.db.maps[ck.map_id]

  -- check active mods
  for _, mod_id in ipairs(ck.active_mods_list) do
    -- make sure mod structure is present
    local mod_namespace_db = state.ensure_mod_namespace(mod_id)

    -- check if any data for mod_namespace_db is present
    local mod_namespace_is_empty = true

    for _, key in ipairs(ck.registries.state_mod_namespace_keys) do
      local mod_entities = ck.registries[key][mod_id]

      if (next(mod_entities) == nil) then
        mod_namespace_db[key] = nil
      else
        mod_namespace_is_empty = false
      end
    end

    if (mod_namespace_is_empty) then
      log.debug("Mod [%s] has nothing to save on a map", mod_id, key)
      state.db.maps[ck.map_id][mod_id] = nil

      goto continue
    end

    -- timers
    -- `maps.id.mod_id.timers` e.g. maps.4.arroyo_expanded.timers
    for tag, timer in pairs(ck.registries.timers[mod_id]) do
      mod_namespace_db.timers[tag] = { created_at = timer.created_at, timer_type = timer.timer_type }
    end

    -- objects
    -- `maps.id.mod_id.objects` e.g. maps.4.arroyo_expanded.objects
    for _, object in pairs(ck.registries.objects[mod_id]) do
      if not object.lua_id or not object.mod_id or not object.tag or object.modified then
        goto continue
      end

      local object_state = mod_namespace_db.objects[object.tag]

      if object.tile then object_state.tile = object:tile() end
      if object.hp   then object_state.hp   = object:hp()   end

      object_state.id = object:id()
      object_state.inventory = object:inventory_table()

      ::continue::
    end

    ::continue::
  end

  -- clear out random encounter maps (unless it's current map)
  for map_id, _ in pairs(state.db.maps) do
    if map_id == ck.map_id then goto continue end

    if not ffi.C.ck_config_is_map_savable(map_id) then
      log.debug("GC: Removing 'saved=no' map[%d]", map_id)
      state.db.maps[map_id] = nil
    end

    ::continue::
  end

  log.header('state.db table:')
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
  local map_id = ck.map_id

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

  return state.db.maps[map_id][mod_id]["objects"][tag]
end

function state.get_state_data(mod_id, map_id, tag)
  log.debug(string.format("mod_id: %s, map_id: %d, tag: %s", mod_id, map_id, tag))
  data = state.get_stored_object_data(mod_id, map_id, tag)

  if data then return data else return {} end
end

return state
