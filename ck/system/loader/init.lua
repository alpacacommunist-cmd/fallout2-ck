-- ck/system/loader/init.lua
local ffi = require('ffi')
local ck  = require('ck')

local utils = require('ck.system.utils')
local sandbox = require('ck.system.loader.sandbox')

local assets    = require('ck.fallout2.assets')

local log = require('ck.system.log').new('loader/init.lua')

local reloadable_mods = {
  "arroyo_expanded",
  "temple_of_trials",
  "test_mod_lib"
}

local active_mod_envs = {}

local loader = {}

local function apply_manifest(manifest)
  if not manifest then return end

  if manifest.assets then
  end

  if manifest.locale then
    -- i18n.register(manifest.id, manifest.locale)
  end
end

loader.handlers = {
  --🎮
  gameplay = function(mod_data, mod_env)
    local mod_id = mod_data.id

    -- inits mod data tables
    ck.registries.init_mod(mod_id)
    -- register mod in backend (to allow setting mod context (current_mod_id))
    ffi.C.ck_dispatcher_add_mod(mod_id)

    local function run_mod_init()
      return mod_env.require(mod_data.keys.init)
    end
    local success = ck.tools.exec_with_mod_context(mod_id, run_mod_init)

    if not success then
      ck.registries.clear_mod(mod_id)
      ffi.C.ck_dispatcher_remove_mod(mod_id)

      return false
    end

    return true
  end,

  --📦
  library = function(mod_data, mod_env)
    local mod_id = mod_data.id

    local success, mod_api = pcall(mod_env.require, mod_data.keys.init)

    if not success then
      log.error("running library '" .. mod_id .. "': " .. tostring(mod_api))
      return false
    end

    if type(mod_api) == "table" then
      local preload_key = ck.tools.mod_preload_key(mod_id)

      package.preload[preload_key] = function()
        return mod_api
      end

      mod_data.keys.preload = preload_key
      log.info("Library '%s' registered to package.preload['%s']", mod_id, preload_key)
    else
      log.warn("Library '%s' loaded but did not return an API table!", mod_id)
    end

    return true
  end
}

function loader.exec_mod(mod_data)
  local manifest = mod_data.manifest
  local mod_id   = mod_data.id

  apply_manifest(manifest)

  -- add mod env
  local mod_env = sandbox.create_env(mod_data)
  active_mod_envs[mod_data.keys.base] = mod_env

  -- exec
  local handler = loader.handlers[manifest.type]
  return handler(mod_data, mod_env)
end

function loader.reload_mods()
  for _, mod_id in ipairs(reloadable_mods) do
    local mod_data = ck.active_mods[mod_id]
    local mod_env  = active_mod_envs[mod_data.keys.base]

    log.header("Reloading mod: %s", mod_id)
    log.info("Clearing out resources for: %s", mod_id)

    ffi.C.ck_critter_reset_prototypes_for_mod(mod_id)
    ffi.C.ck_registry_clear_for_mod(mod_id)
    ffi.C.ck_map_clear_camera_borders_for_mod(mod_id)
    -- TODO: remove from reload sequence
    -- ffi.C.ck_config_clear_mod_patches(mod_id)

    -- clear lua registries
    ck.registries.clear_mod(mod_id)

    if mod_env and mod_env.package and mod_env.package.loaded then
      for module_name in pairs(mod_env.package.loaded) do
        log.info("Unloaded from mod cache: " .. module_name)
      end

      mod_env.package.loaded = {}
    end

    if mod_data.manifest.type == 'library' then
      package.preload[mod_data.keys.preload] = nil
      log.info("[preload] Unloaded: " .. mod_data.keys.preload)
    end

    if loader.exec_mod(mod_data) then
      if mod_data.manifest.type == 'gameplay' then
        ffi.C.ck_dispatcher_emit_for_mod(mod_id, "map_enter")
        ffi.C.ck_dispatcher_emit_for_mod(mod_id, "onModReload")
      end
    end
  end

  log.info("Reload complete!")
end

return loader
