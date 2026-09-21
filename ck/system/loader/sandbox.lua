-- ck/fallout2/loader/sandbox.lua
local ck = require('ck')
local utils = require('ck.system.utils')
local log   = ck.log.new('sandbox.lua')

local _G_require = _G.require

local sandbox = {}

local safe_globals = {
  print = print, pairs = pairs, ipairs = ipairs, next = next,
  tostring = tostring, tonumber = tonumber, type = type,
  select = select, pcall = pcall, xpcall = xpcall, error = error,
  assert = assert,

  string = string, table = table, math = math, bit = bit
}

sandbox.handlers = {
  -- 📦 libs
  library = function(env, mod_folder, manifest)
  end,

  -- 🎮 gameplay
  gameplay = function(env, mod_folder, manifest)
  end
}

local function resolve_relative_path(target_name)
  if target_name:sub(1, 1) ~= "." then
    return target_name
  end

  -- debug.getinfo(3, "S") checks 3 levels up in call stack:
  -- 1 - resolve_relative_path
  -- 2 - env.require
  -- 3 - mod file calling require()
  local info = debug.getinfo(3, "S")
  if info and info.source and info.source:sub(1, 1) == "@" then
    -- info.source: "@../mods/arroyo_expanded/dialogs.lua"
    local current_file = info.source:sub(2)

    -- file path to lua notation
    local current_dir = current_file:match("(.*[/\\])") or ""
    current_dir = current_dir:gsub("[/\\]", "."):gsub("^%.+", ""):gsub("%.$", "")

    -- compile path: "mods.arroyo_expanded" + ".locale.ru"
    local full_path = current_dir .. target_name
    return full_path
  end

  return target_name
end

function sandbox.create_env(mod_data)
  local mod_package = {
    loaded = {},        -- module cache
    path = package.path -- module paths
  }

  local env = setmetatable({
    package = mod_package,
  }, { __index = safe_globals })

  local manifest = mod_data.manifest

  env.__manifest = manifest
  env.__mod_id   = manifest.id

  env.log = ck.log.new("MOD: " .. manifest.name)

  ---------------------------------------------------------------
  ------ require
  ---------------------------------------------------------------

  function env.require(target_name)
    if target_name == "ck.init" or target_name == "ck" then return ck.public end
    if target_name == 'ffi' then return nil end

    -- allow relative paths (eg '.locale.ru' -> 'mods.arroyo_expanded.locale.ru')
    target_name = resolve_relative_path(target_name)

    if mod_package.loaded[target_name] then
      return mod_package.loaded[target_name]
    end

    -- system module (ck.*)
    local is_system_module = target_name:match("^ck%.")

    if is_system_module then
      local success, module = pcall(_G.require, target_name)
      if not success then error(module) end

      local result_to_cache = module

      if type(module) == "table" and module.public then
        log.debug('=============')
        log.debug('returning public table: %s', target_name)
        log.debug('=============')

        result_to_cache = module.public
      end

      mod_package.loaded[target_name] = result_to_cache
      return result_to_cache
    end

    -- mod's local file (eg mods.arroyo_expanded.dialogs)
    -- find file
    local filepath, error = package.searchpath(target_name, package.path)
    -- if not filepath then
    --   -- (just in case)
    --   local success, res = pcall(_G.require, target_name)
    --   if success then return res end
    --   error(string.format("Module '%s' not found:\n%s", target_name, err))
    -- end

    -- read and compile using mod env
    local content = utils.read_file(filepath, log)
    local chunk, chunk_err = utils.compile_chunk(content, filepath)
    if not chunk then
      error(string.format("Syntax error in '%s':\n%s", target_name, chunk_err))
    end

    -- (JIT-friendly, chunk wasn't executed yet)
    setfenv(chunk, env)

    -- exec chunk
    local success, result = xpcall(chunk, debug.traceback)
    if not success then
      error(string.format("Runtime error in '%s':\n%s", target_name, result))
    end

    package.loaded[target_name] = result or true
    return package.loaded[target_name]
  end

  local decorator = sandbox.handlers[manifest.type]
  if decorator then
    decorator(env, mod_folder, manifest)
  end

  return env
end

return sandbox
