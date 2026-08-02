-- ck/system/loader/init.lua
local ffi = require("ffi")

local sandbox   = require('ck.system.loader.sandbox')
local events    = require('ck.system.events')

local rendering = require('ck.fallout2.rendering')
local assets    = require('ck.fallout2.assets')
local i18n      = require('ck.fallout2.i18n')
local state     = require('ck.fallout2.state')
local objects   = require('ck.fallout2.objects')

local log = ck.log.new('CK Loader')

local reloadable_mods = {
  "arroyo_expanded",
  "temple_of_trials"
}

local loader = {}

local function load_manifest(mod_id)
  local key = 'mods.' .. mod_id .. '.mod'

  local ok, manifest = pcall(require, key)

  if not ok or type(manifest) ~= 'table' then
    log.warn("WARNING: no manifest for " .. mod_id)
    return nil
  end

  return manifest
end

local function apply_manifest(manifest)
  if not manifest then return end

  if manifest.assets then
  end

  if manifest.locale then
    i18n.register(manifest.id, manifest.locale)
  end
end

function loader.load_and_init_mod(mod_id)
  local manifest = load_manifest(mod_id)
  apply_manifest(manifest)

  local mod_key = 'mods.' .. mod_id .. ".init"
  local file_path = "../" .. mod_key:gsub("%.", "/") .. ".lua"

  local file = io.open(file_path, "r")
  if not file then
    log.error("Cannot open mod file: " .. file_path)
    return
  end
  local content = file:read("*a")
  file:close()

  -- compile file into function
  local mod_init_fn, err = loadstring(content, "@" .. file_path)
  if not mod_init_fn then
    log.error("compiling mod '" .. mod_id .. "': " .. tostring(err))
    return
  end

  local mod_env = sandbox.create_env(mod_id, manifest)
  setfenv(mod_init_fn, mod_env)

  -- exec mod
  local success, run_err = pcall(mod_init_fn)
  if not success then
    log.error("running mod '" .. mod_id .. "': " .. tostring(run_err))
  end

  return manifest
end

function loader.reload_mods()
  log.info("Reloading mods...")

  rendering.clear()

  for _, mod_id in ipairs(reloadable_mods) do
    local target_prefix = "mods." .. mod_id

    events.clear_for_mod(mod_id)
    objects.clear_for_mod(mod_id)
    state.clear_for_mod(mod_id)

    ffi.C.ck_registry_destroy_objects_for_mod(mod_id)
    ffi.C.ck_config_clear_mod_patches(mod_id)
    ffi.C.ck_map_clear_camera_borders_for_mod(mod_id)

    for mod_name in pairs(package.loaded) do
      if mod_name:match("^" .. target_prefix) then
        package.loaded[mod_name] = nil
        log.info("Unloaded: " .. mod_name)
      end
    end
  end

  for _, mod_id in ipairs(reloadable_mods) do
    log.info("Reloading: " .. mod_id)
    local success = ffi.C.ck_dispatcher_load_mod(mod_id)

    if success then
      ffi.C.ck_dispatcher_emit_for_mod(mod_id, "onMapEnter")
      ffi.C.ck_dispatcher_emit_for_mod(mod_id, "onModReload")
    end
  end

  log.info("Reload complete!")
end

_G["ckReloadMods"] = loader.reload_mods

return loader
