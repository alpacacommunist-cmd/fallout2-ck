-- ck/fallout2/loader/sandbox.lua
local utils = require('ck.system.utils')
local core_events = require('ck.system.events')
local log = ck.log.new('sandbox.lua')

local sandbox = {}

sandbox.handlers = {
  -- 📦 libs
  library = function(env, mod_folder, manifest)
    -- Nothing here for now
    -- Just the basic env
  end,

  -- 🎮 gameplay
  gameplay = function(env, mod_folder, manifest)
    -- events override
    env.events = setmetatable({}, { __index = core_events })

    function env.events.on(event_name, callback)
      core_events.register(manifest.id, event_name, callback)
    end
  end
}

function sandbox.create_env(mod_data)
  local env = setmetatable({}, { __index = _G })
  local manifest = mod_data.manifest

  env.__manifest = manifest
  env.__mod_id   = manifest.id

  env.log = ck.log.new("MOD: " .. manifest.name)

  ---------------------------------------------------------------
  ------ require
  ---------------------------------------------------------------
  function env.require(target_name)
    -- mod libs
    if target_name:match("^ck%.libs%.") then
      local lib_id = target_name:gsub("^ck%.libs%.", "")
      local library = ck.loaded_libs[lib_id]

      if not library then
        error(string.format("Runtime Error: Core library '%s' requested by mod '%s' is not available!", lib_id, env.__mod_id))
      end

      return library
    end

    -- relative requires (as in require('.outskirts') instead of require('temple_of_trials.outskirts')
    if target_name:sub(1, 1) == "." then
      target_name = mod_data.paths.base .. target_name
    end

    -- replace explicit require from mod to proxied version
    if target_name == "ck.fallout2.events" and env.events then
      return env.events
    end

    -- check if module is loaded
    if package.loaded[target_name] then
      return package.loaded[target_name]
    end

    -- search in standard path
    local loader_fn, err = package.searchpath(target_name, package.path)
    if not loader_fn then
      -- sandboxed require found nothing, try global
      return _G.require(target_name)
    end

    -- read and compile
    local content = utils.read_file(loader_fn, log)

    local chunk, chunk_err = loadstring(content, "@" .. loader_fn)
    if not chunk then
      local trace = debug.traceback(string.format("Syntax error compiling module '%s':\n%s", target_name, chunk_err), 2)
      error(trace)
    end

    setfenv(chunk, env)

    local success, result = xpcall(chunk, debug.traceback)
    if not success then
      error(string.format("Runtime error executing module '%s':\n%s", target_name, result))
    end

    -- exec and cache the result
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
