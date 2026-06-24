-- ck/fallout2/loader/sandbox.lua
local core_events   = require('ck.fallout2.events')
local i18n          = require('ck.fallout2.i18n')
local core_critters = require('ck.fallout2.objects.critters')
local core_state    = require('ck.fallout2.state')
local core_quests   = require('ck.fallout2.quests')

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

    -- replace explicit require from mod to proxied version
    if target_name == "ck.fallout2.state" then return env.state end
    if target_name == "ck.fallout2.events" then return env.events end

    -- smart proxies instead of system modules
    if target_name == "ck.fallout2.objects.critters" then
      return env.critters
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
  -- env.state
  ---------------------------------------------------------------------

  env.state = setmetatable({}, { __index = core_state })

  function env.state.track(object_instance, options)
    options = options or {}

    return core_state.track_internal(manifest_table.id, object_instance, options)
  end

  ---------------------------------------------------------------------
  -- env.critters
  ---------------------------------------------------------------------

  env.critters = setmetatable({}, { __index = core_critters })

  function env.critters.register(tag, pid, tile, config)
    return core_critters.register(tag, pid, tile, config, manifest_table.id)
  end
  function env.critters.create(pid, tile, config)
    return core_critters.create(pid, tile, config, manifest_table.id)
  end

  ---------------------------------------------------------------------
  -- env.quests
  ---------------------------------------------------------------------

  env.quests = setmetatable({}, { __index = core_quests })

  function env.quests.register(quest_id, config)
    return core_quests.register_internal(manifest_table.id, quest_id, config)
  end
  function env.quests.set(quest_id, status_value)
    return core_quests.set_internal(manifest_table.id, quest_id, status_value)
  end
  function env.quests.get(quest_id)
    return core_quests.get_internal(manifest_table.id, quest_id)
  end

  return env
end

return M
