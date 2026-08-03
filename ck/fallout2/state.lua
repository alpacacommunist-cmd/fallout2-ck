-- ck/fallout2/state.lua
local ffi = require("ffi")

local map = require('ck.fallout2.map')

local state = {}

local log   = ck.log.new('state.lua')
local utils = require('ck.system.utils')

local db = {
  global = {},
  maps   = {}
}

local tracked_objects = {}

-- gets marshalld json -> lua from backend
function state.sync_load(loaded_db)
  db = loaded_db or { global = {}, maps = {} }
  db.global = db.global or {}
  db.maps   = db.maps or {}
end

-- returns lua, backend marshalls it to json and saves
function state.sync_save()
  local current_map_id = map.get_id()

  if current_map_id ~= -1 then
    db.maps[current_map_id] = db.maps[current_map_id] or {}

    for lua_id, entry in pairs(tracked_objects) do
      db.maps[current_map_id][entry.mod_id] = db.maps[current_map_id][entry.mod_id] or {}
      db.maps[current_map_id][entry.mod_id][entry.tag] = db.maps[current_map_id][entry.mod_id][entry.tag] or {}

      local current_data = db.maps[current_map_id][entry.mod_id][entry.tag]

      if entry.object.tile then
        current_data.tile =  entry.object:tile()
      end

      if entry.object.hp then
        db.maps[current_map_id][entry.mod_id][entry.tag].hp = entry.object:hp()
      end
    end
  end

  return db
end

function state.track(object_instance, options)
  options = options or {}

  if not object_instance or not object_instance.tag then
    log.error("Cannot track object without a valid instance or tag!")

    return
  end

  local mod_id = object_instance.mod_id

  if not mod_id or mod_id == "unknown" then
    log.error(string.format("Object '%s' has an unknown mod_id, can't track!", object_instance.tag))
    return
  end

  local lua_id = object_instance.lua_id
  local interval_seconds = options.save_interval_seconds or 5
  local interval_ticks = interval_seconds * 10

  tracked_objects[lua_id] = {
    mod_id = mod_id, tag = object_instance.tag, object = object_instance,
    interval = interval_ticks, next_tick = 0
  }

  log.info(string.format("Started tracking object '%s' for mod '%s'", object_instance.tag, mod_id))
end

function state.untrack(lua_id)
  if tracked_objects[lua_id] then
    local tag = tracked_objects[lua_id].tag
    local mod_id = tracked_objects[lua_id].mod_id

    tracked_objects[lua_id] = nil

    log.info(string.format("Stopped tracking object '%s' for mod '%s' (Object Destroyed)", tostring(tag), tostring(mod_id)))
  end
end

function state.update_tracked_objects(current_ticks)
  local map_id = map.get_id()

  if map_id  == -1 then return end

  for lua_id, entry in pairs(tracked_objects) do
    if current_ticks >= entry.next_tick then
      entry.next_tick = current_ticks + entry.interval

      local tile = entry.object:tile()

      db.maps[map_id] = db.maps[map_id] or {}
      db.maps[map_id][entry.mod_id] = db.maps[map_id][entry.mod_id] or {}
      db.maps[map_id][entry.mod_id][entry.tag] = db.maps[map_id][entry.mod_id][entry.tag] or {}

      db.maps[map_id][entry.mod_id][entry.tag].tile = tile

      -- if entry.obj.type == "critter" then ...
    end
  end
end

function state.clear_for_mod(mod_name)
  for lua_id, entry in pairs(tracked_objects) do
    if entry.mod_id == mod_name then
      tracked_objects[lua_id] = nil
    end
  end

  log.info("Cleared tracked objects cache for mod: " .. mod_name)
end

function state.clear_tracked_objects()
  tracked_objects = {}
  log.info("Tracked objects registry cleared")
end

local function get_mod_storage(section, mod_id)
  if not mod_id then return nil end
  local map_id = map.get_id()

  if section == "global" then
    db.global[mod_id] = db.global[mod_id] or {}

    return db.global[mod_id]
  elseif section == "maps" then
    if map_id == -1 then return nil end

    db.maps[map_id] = db.maps[map_id] or {}
    db.maps[map_id][mod_id] = db.maps[map_id][mod_id] or {}
    return db.maps[target_map][m_id]
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
  db.global[mod_id] = db.global[mod_id] or {}
  db.global[mod_id][sub_section] = db.global[mod_id][sub_section] or {}
  db.global[mod_id][sub_section][key] = value
end

function state.get_global(mod_id, sub_section, key)
  if db.global[mod_id] and db.global[mod_id][sub_section] then
    return db.global[mod_id][sub_section][key]
  end
  return nil
end

function state.get_stored_object_data(mod_id, map_id, tag)
  -- utils.print_table(db, log)

  if db.maps[map_id] and db.maps[map_id][mod_id] and db.maps[map_id][mod_id][tag] then
    return db.maps[map_id][mod_id][tag]
  end

  return nil
end

function state.get_state_data(mod_id, map_id, tag)
  log.debug(string.format("mod_id: %s, map_id: %d, tag: %s", mod_id, map_id, tag))
  data = state.get_stored_object_data(mod_id, map_id, tag)

  if data then return data else return {} end
end

function state.get_state_tile(mod_id, map_id, tag)
  log.debug(string.format("mod_id: %s, map_id: %d, tag: %s", mod_id, map_id, tag))
  data = state.get_stored_object_data(mod_id, map_id, tag)

  if data then return data.tile else return -1 end
end

return state
