-- ck/system/loader/init.lua
local ffi = require("ffi")

local utils = require('ck.system.utils')
local sandbox = require('ck.system.loader.sandbox')

local assets    = require('ck.fallout2.assets')
local i18n      = require('ck.fallout2.i18n')

local registries = require('ck.system.registries')
local mod_tools  = require('ck.system.mod_tools')

local log = ck.log.new('loader/init.lua')

local reloadable_mods = {
  "arroyo_expanded",
  "temple_of_trials"
}

local loader = {}

local function apply_manifest(manifest)
  if not manifest then return end

  if manifest.assets then
  end

  if manifest.locale then
    i18n.register(manifest.id, manifest.locale)
  end
end

loader.handlers = {
  --🎮
  gameplay = function(mod_data, mod_init_fn)
    local mod_id = mod_data.id

    registries.init_mod(mod_id)
    ffi.C.ck_dispatcher_add_mod(mod_id)

    local success = mod_tools.exec_with_mod_context(mod_id, mod_init_fn)

    if not success then
      registries.clear_mod(mod_id)
      ffi.C.ck_dispatcher_remove_mod(mod_id)

      return false
    end

    return true
  end,

  --📦
  library = function(mod_data, mod_init_fn)
    local mod_id = mod_data.id
    local success, result = pcall(mod_init_fn)

    if not success then
      log.error("running library '" .. mod_id .. "': " .. tostring(result))
      return false
    end

    if type(result) == "table" then
      ck.loaded_libs[mod_id] = result
      log.info("Library '%s' registered to ck.libs.%s", mod_id, mod_id)
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

  -- filepath of mod's .init
  local init_file_path = mod_tools.key_to_path(mod_data.keys.init)

  -- read file
  local content = utils.read_file(init_file_path, log)

  -- create chunk
  local mod_init_fn, err = loadstring(content, "@" .. init_file_path)
  if not mod_init_fn then
    log.error("compiling mod '" .. mod_id .. "': " .. tostring(err))
    return false
  end

  setfenv(mod_init_fn, mod_env)

  -- exec
  local handler = loader.handlers[manifest.type]
  return handler(mod_data, mod_init_fn)
end

function loader.reload_mods()
  for _, mod_id in ipairs(reloadable_mods) do
    local mod_data = ck.active_mods[mod_id]

    log.header("Reloading mod: %s", mod_id)
    log.info("Clearing out resources for: %s", mod_id)

    ffi.C.ck_critter_reset_prototypes_for_mod(mod_id)
    ffi.C.ck_registry_clear_for_mod(mod_id)
    ffi.C.ck_map_clear_camera_borders_for_mod(mod_id)
    -- TODO: remove from reload sequence
    -- ffi.C.ck_config_clear_mod_patches(mod_id)

    -- clear lua registries
    registries.clear_mod(mod_id)

    -- unload requires (submodules)
    for module_name in pairs(package.loaded) do
      if module_name:match("^" .. mod_data.keys.base) then
        package.loaded[module_name] = nil
        log.info("Unloaded: " .. module_name)
      end
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
