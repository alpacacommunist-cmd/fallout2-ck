-- ck/fallout2/loader/sandbox.lua
local core_events = require('ck.system.events')
local log = ck.log.new('CK Events Sandbox')

local sandbox = {}

function sandbox.create_env(mod_folder, manifest_table)
  local env = setmetatable({}, { __index = _G })
  local is_library = manifest_table.type and manifest_table.type == "library"

  env.__manifest = manifest_table
  env.__mod_id   = manifest_table.id

  env.log = ck.log.new("MOD: " .. manifest_table.name)

  ---------------------------------------------------------------
  ------ require
  ---------------------------------------------------------------
  function env.require(mod_name)
    local target_name = mod_name

    if mod_name:match("^ck%.libs%.") then
      local lib_id = mod_name:gsub("^ck%.libs%.", "")
      local library = ck.loaded_libs[lib_id]

      if not library then
        error(string.format("Runtime Error: Core library '%s' requested by mod '%s' is not available!", lib_id, env.__mod_id))
      end

      return library
    end

    -- relative requires (as in require('.outskirts') instead of require('temple_of_trials.outskirts')
    if mod_name:sub(1, 1) == "." then
      target_name = "mods." .. mod_folder .. mod_name
    end

    -- replace explicit require from mod to proxied version
    if not is_library then
      if target_name == "ck.fallout2.events" then return env.events end
    end

    -- check if module is loaded
    if package.loaded[target_name] then
      return package.loaded[target_name]
    end

    -- search in standard path
    local loader_fn, err = package.searchpath(target_name, package.path)
    if not loader_fn then
      -- sandboxed require found nothing, try global
      return _G.require(mod_name)
    end

    -- read and compile
    local file = io.open(loader_fn, "r")
    local content = file:read("*a")
    file:close()

    local chunk, chunk_err = loadstring(content, "@" .. loader_fn)
    if not chunk then error(chunk_err) end

    -- same sandbox
    setfenv(chunk, env)

    -- exec and cache the result
    local result = chunk()
    package.loaded[target_name] = result or true
    return package.loaded[target_name]
  end

  ---------------------------------------------------------------
  ------ events
  ---------------------------------------------------------------

  if not is_library then
    env.events = setmetatable({}, { __index = core_events })

    function env.events.on(event_name, callback)
      core_events.register(manifest_table.id, event_name, callback)
    end
  end

  return env
end

return sandbox
