-- ck/fallout2/state.lua
local ffi   = require("ffi")
local utils = require('ck.system.utils')

local state = {}
local log   = ck.log.new('state.lua')

local db_init_state = { player = { knowledge = {} }, global = {}, maps = {} }
state.db = { player = db_init_state.player, global = db_init_state.global, maps = db_init_state.maps }

-- gets marshalld json -> lua from backend
function state.sync_load(loaded_db)
  state.db = loaded_db

  state.db.player = state.db.player or db_init_state.player
  state.db.global = state.db.global or db_init_state.global
  state.db.maps   = state.db.maps or db_init_state.maps

  -- utils.print_table(state.db.proto_list, log)
end

-- returns lua, backend marshalls it to json and saves
function state.sync_save()
  local current_map_id = ffi.C.ck_map_get_id()
  if current_map_id == -1 then return state.db end

  state.db.maps[current_map_id] = state.db.maps[current_map_id] or {}
  local current_map = state.db.maps[current_map_id]

  local objects = require('ck.fallout2.objects')

  for _, object in pairs(objects.registry) do
    if not object.lua_id or not object.mod_id or not object.tag then
      goto continue
    end

    current_map[object.mod_id] = current_map[object.mod_id] or {}
    current_map[object.mod_id][object.tag] = current_map[object.mod_id][object.tag] or {}

    local object_state = current_map[object.mod_id][object.tag]

    if object.tile then object_state.tile = object:tile() end
    if object.hp   then object_state.hp   = object:hp()   end

    object_state.id = object:id()
    object_state.inventory = object:inventory_table()

    ::continue::
  end

  return state.db
end

function state.get_proto_list()
  return state.db.proto_list
end

function state.receive_proto_list(data_address, size)
  local protos = ffi.cast("CustomProtoLuaView*", data_address)

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
  if state.db.maps[map_id] and state.db.maps[map_id][mod_id] and state.db.maps[map_id][mod_id][tag] then
    return state.db.maps[map_id][mod_id][tag]
  end

  return nil
end

function state.get_state_data(mod_id, map_id, tag)
  log.debug(string.format("mod_id: %s, map_id: %d, tag: %s", mod_id, map_id, tag))
  data = state.get_stored_object_data(mod_id, map_id, tag)

  if data then return data else return {} end
end

return state
