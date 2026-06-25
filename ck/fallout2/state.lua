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

-- gets mod_id of the caller
local function get_caller_mod_id()
  for level = 2, 10 do
    local success, env = pcall(getfenv, level)

    if not success or not env then break end

    if env.mod_id then return env.mod_id end
  end

  return "unknown"
end

-- helper function for printing nested stuff
local function print_table(t, indent)
  indent = indent or 0
  for k, v in pairs(t) do
    local formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      log.debug(formatting)
      print_table(v, indent + 1)
    else
      log.debug(formatting .. tostring(v))
    end
  end
end

-- serialize
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

-- global callbacks - onGameSave
function ckOnGameSave(path)
  local current_map_id = map.get_id()

  if current_map_id ~= -1 then
    for lua_id, entry in pairs(tracked_objects) do
      local tile = entry.obj:tile()
      db.maps[current_map_id] = db.maps[current_map_id] or {}
      db.maps[current_map_id][entry.mod_id] = db.maps[current_map_id][entry.mod_id] or {}
      db.maps[current_map_id][entry.mod_id][entry.tag] = db.maps[current_map_id][entry.mod_id][entry.tag] or {}
      db.maps[current_map_id][entry.mod_id][entry.tag].tile = tile
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

-- global callbacks - onGameLoad
function ckOnGameStateLoad(path)
  last_save_path = path:gsub("\\", "/")
  log.info("Cached save game state path: " .. last_save_path)

  state.load_from_cache()
end

function state.track(object_instance, options)
  mod_id = get_caller_mod_id()

  if mod_id == "unknown" then
    log.error("Unknown mod_id, can't track object")

    return
  end

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
  local map_id = map.get_id()

  if map_id  == -1 then return end

  for lua_id, entry in pairs(tracked_objects) do
    if current_ticks >= entry.next_tick then
      entry.next_tick = current_ticks + entry.interval

      local tile = entry.obj:tile()

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

function state.load_from_cache()
  if not last_save_path then return false end

  local file = io.open(last_save_path, "r")
  if not file then
    log.info("No state file found in cache path (New game?): " .. last_save_path)
    return false
  end

  local content = file:read("*a")
  file:close()

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
  print_table(db)
  if db.maps[map_id] and db.maps[map_id][mod_id] and db.maps[map_id][mod_id][tag] then
    return db.maps[map_id][mod_id][tag]
  end

  return nil
end

return state
