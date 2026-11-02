-- ck/fallout2/loader/sandbox.lua
local ck = require('ck')
local utils = require('ck.system.utils')
local log = ck.log.new('sandbox.lua')

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
  library = function(env, mod_data)
  end,

  -- 🎮 gameplay
  gameplay = function(env, mod_data)
  end
}

function sandbox.create_env(mod_data)
  local manifest = mod_data.manifest
  local mod_folder = ck.tools.mod_directory(manifest.id)

  local mod_package = {
    loaded = {}, -- module cache
    path = package.path
  }

  local env = setmetatable({
    package = mod_package,
  }, { __index = safe_globals })

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
    if target_name:sub(1, 1) == "." then
      target_name = "mods." .. env.__mod_id .. target_name
    end

    if mod_package.loaded[target_name] then
      return mod_package.loaded[target_name]
    end
    -- system module (ck.*)
    local is_system_module = target_name:match("^ck%.")

    if is_system_module then
      local success, module = pcall(_G.require, target_name)
      if not success then _G.error(module) end

      local result_to_cache = module

      if type(module) == "table" and module.public then
        log.debug('returning public table: %s', target_name)
        result_to_cache = module.public
      end

      mod_package.loaded[target_name] = result_to_cache
      return result_to_cache
    end

    -- mod's local file (eg mods.arroyo_expanded.dialogs)
    local filepath, error = package.searchpath(target_name, mod_package.path)

    -- read and compile using mod env
    local content = utils.read_file(filepath, log)
    local chunk, chunk_err = utils.compile_chunk(content, filepath, env)
    if not chunk then
      _G.error(string.format("Syntax error in '%s':\n%s", target_name, chunk_err))
    end

    -- exec chunk
    local success, result = xpcall(chunk, debug.traceback)
    if not success then
      _G.error(string.format("Runtime error in '%s':\n%s", target_name, result))
    end

    mod_package.loaded[target_name] = result or true
    return mod_package.loaded[target_name]
  end

  local decorator = sandbox.handlers[manifest.type]
  if decorator then
    decorator(env, mod_data)
  end

  return env
end

return sandbox
