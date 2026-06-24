-- ck/fallout2/loader/sandbox.lua
local core_events   = require('ck.fallout2.events')
local i18n          = require('ck.fallout2.i18n')
local core_critters = require('ck.fallout2.objects.critters')

local log = ck.log.new('CK Events Sandbox')

local M = {}

function M.create_env(mod_folder, manifest_table)
  local env = setmetatable({}, { __index = _G })

  env.manifest = manifest_table

  ---------------------------------------------------------------
  ------ require
  ---------------------------------------------------------------
  function env.require(mod_name)
    local target_name = mod_name

    -- relative requires (as in require('.outskirts') instead of require('temple_of_trials.outskirts')
    if mod_name:sub(1, 1) == "." then
      target_name = "mods." .. mod_folder .. mod_name
    end

    -- smart proxies instead of system modules
    if target_name == "ck.fallout2.objects.critters" then
      return {
        register = function(tag, pid, tile, config)
          config = config or {}
          config.mod_id = manifest_table.id
          return core_critters.register(tag, pid, tile, config)
        end,

        create = core_critters.create
      }
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
  ------ context log
  ---------------------------------------------------------------

  env.log = ck.log.new(manifest_table.name)

  ---------------------------------------------------------------
  ------ events
  ---------------------------------------------------------------

  env.events = setmetatable({}, { __index = core_events })

  function env.events.on(event_name, callback)
    if not core_events.listeners[event_name] then
      log.warn(string.format("[CK Sandbox] [%s] Warning: unknown event '%s'", mod_folder, tostring(event_name)))
      return
    end

    table.insert(core_events.listeners[event_name], {
      mod = mod_folder,
      fn  = callback
    })
  end

  ---------------------------------------------------------------
  ------ I18n
  ---------------------------------------------------------------

  env.i18n = setmetatable({}, { __index = i18n })

  function env.i18n.t(key, ...)
    if select('#', ...) == 0 or type(key) == 'string' and type(select(1, ...)) ~= 'string' then
      return i18n.t(mod_folder, key, ...)
    end
    return i18n.t(key, ...)
  end

  ---------------------------------------------------------------------
  -- env.state = ...
  ---------------------------------------------------------------------

  return env
end

return M
