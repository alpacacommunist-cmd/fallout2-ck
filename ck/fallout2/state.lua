-- ck/fallout2/state.lua
local map = require('ck.fallout2.map')

local state = {}

local log = ck.log.new('CK State')

local db = {
  global = {},
  maps   = {}
}

local last_save_path  = nil
local tracked_objects = {}

function state.track_internal(mod_id, object_instance, options)
  if not object_instance or not object_instance.tag then
    log.error("Cannot track object without a valid instance or tag!")

    return
  end

  local lua_id = object_instance.id
  local interval_seconds = options.save_interval_seconds or 5

  local interval_ticks = interval_seconds * 10

  tracked_objects[lua_id] = {
    mod_id = mod_id,
    tag = object_instance.tag,
    obj = object_instance,
    interval = interval_ticks,
    next_tick = 0
  }

  log.info(string.format("Started tracking object '%s' for mod '%s'", object_instance.tag, mod_id))
end

function state.update_tracked_objects(current_ticks)
  local current_map_id = map.get_id()

  if current_map_id == -1 then return end
  if tracked_object == nil then return end

  for lua_id, entry in pairs(tracked_objects) do
    if current_ticks >= entry.next_tick then
      entry.next_tick = current_ticks + entry.interval

      local current_tile = entry.obj:tile()

      db.maps[current_map_id] = db.maps[current_map_id] or {}
      db.maps[current_map_id][entry.mod_id] = db.maps[current_map_id][entry.mod_id] or {}
      db.maps[current_map_id][entry.mod_id][entry.tag] = db.maps[current_map_id][entry.mod_id][entry.tag] or {}

      db.maps[current_map_id][entry.mod_id][entry.tag].tile = current_tile

      -- if entry.obj.type == "critter" then ... в зависимости от класса
    end
  end
end

function state.clear_tracked_objects()
  tracked_objects = {}
  log.info("Tracked objects registry cleared")
end

function state.load_from_cache()
  if not last_save_path then return false end

  local f = io.open(last_save_path, "r")
  if not f then
    log.info("No state file found in cache path (New game?): " .. last_save_path)
    return false
  end

  local content = f:read("*a")
  f:close()

  local fn, err = load(content)
  if fn then
    db = fn() or { global = {}, maps = {} }
    db.global = db.global or {}
    db.maps = db.maps or {}
    log.info("Data loaded and initialized from cache path!")
    return true
  else
    log.error("Error parsing cached state file: " .. tostring(err))
    return false
  end
end

local function get_mod_storage(section, mod_id)
  local m_id = mod_id or current_mod_id
  if not m_id then return nil end

  if section == "global" then
    db.global[m_id] = db.global[m_id] or {}
    return db.global[m_id]
  elseif section == "maps" then
    local target_map = current_map_id
    if target_map == -1 then return nil end
    db.maps[target_map] = db.maps[target_map] or {}
    db.maps[target_map][m_id] = db.maps[target_map][m_id] or {}
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

local function print_table(t, indent)
    indent = indent or 0
    for k, v in pairs(t) do
        local formatting = string.rep("  ", indent) .. k .. ": "
        if type(v) == "table" then
            print(formatting)
            print_table(v, indent + 1)
        else
            print(formatting .. tostring(v))
        end
    end
end

local function serialize(val)
  local t = type(val)
  if t == "number"  then return tostring(val) end
  if t == "boolean" then return tostring(val) end
  if t == "string"  then return string.format("%q", val) end
  if t == "table" then
    local parts = {}
    for k, v in pairs(val) do
      if type(v) ~= "function" and type(v) ~= "userdata" and type(v) ~= "cdata" then
        local key = type(k) == "string" and string.format("[%q]", k) or (type(k) == "number" and "[" .. tostring(k) .. "]")
        if key then table.insert(parts, key .. "=" .. serialize(v)) end
      end
    end
    return "{" .. table.concat(parts, ",") .. "}"
  end
  return "nil"
end

function ckOnGameSave(path)
  local current_map_id = map.get_id()

  if current_map_id ~= -1 then
    for lua_id, entry in pairs(tracked_objects) do
      local current_tile = entry.obj:tile()
      db.maps[current_map_id] = db.maps[current_map_id] or {}
      db.maps[current_map_id][entry.mod_id] = db.maps[current_map_id][entry.mod_id] or {}
      db.maps[current_map_id][entry.mod_id][entry.tag] = db.maps[current_map_id][entry.mod_id][entry.tag] or {}
      db.maps[current_map_id][entry.mod_id][entry.tag].tile = current_tile
    end
  end

  path = path:gsub("\\", "/")
  last_save_path = path
  local file = io.open(path, "w")
  if not file then return end
  file:write("return " .. serialize(db) .. "\n")
  file:close()
  log.info("Saved database to: " .. path)
end

function ckOnGameStateLoad(path)
  last_save_path = path:gsub("\\", "/")
  log.info("Cached save game state path: " .. last_save_path)

  state.load_from_cache()
end

function state.get_stored_object_data(mod_id, map_id, tag)
  print_table(db)
  if db.maps[map_id] and db.maps[map_id][mod_id] and db.maps[map_id][mod_id][tag] then
    return db.maps[map_id][mod_id][tag]
  end

  return nil
end

return state
