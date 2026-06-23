-- ck/fallout2/state.lua
local state = {}

local log = ck.log.new('CK State')

local db = {
  global = {},
  maps   = {}
}

local current_map_id = -1
local current_mod_id = nil
local last_save_path = nil

function state.set_current_context(mod_id, map_id)
  if mod_id then current_mod_id = mod_id end
  if map_id then current_map_id = map_id end
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
end

return state
