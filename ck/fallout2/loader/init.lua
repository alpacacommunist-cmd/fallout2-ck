-- ck/fallout2/loader.lua
local ffi = require("ffi")

ffi.cdef[[
  void ck_registry_destroy_objects_for_mod(const char* target_mod_id);
  void ck_registry_clear();
]]

local rendering = require('ck.fallout2.rendering')
local events    = require('ck.fallout2.events')
local assets    = require('ck.fallout2.assets')
local i18n      = require('ck.fallout2.i18n')
local state     = require('ck.fallout2.state')
local sandbox   = require('ck.fallout2.loader.sandbox')
local objects   = require('ck.fallout2.objects')

local log = ck.log.new('CK Loader')

local reloadable_mods = {
  "arroyo_expanded",
  "temple_of_trials"
}

local loader = { MODS = {} }

local function load_manifest(mod_folder)
  local key = 'mods.' .. mod_folder .. '.mod'

  local ok, manifest = pcall(require, key)

  if not ok or type(manifest) ~= 'table' then
    log.warn("WARNING: no manifest for " .. mod_folder)
    return nil
  end

  return manifest
end

local function apply_manifest(manifest)
  if not manifest then return end

  if manifest.assets then
    assets.register(manifest.id, manifest.assets)
  end

  if manifest.locale then
    i18n.register(manifest.id, manifest.locale)
  end

  if manifest.locations then
    for _, loc in ipairs(manifest.locations) do
      local map_file_upper = loc.map_file:upper()

        ckRegisterLocation(
          manifest.id, manifest.maps,

          loc.name, loc.sub_name or "Entrance",
          map_file_upper,
          loc.music or "07desert",
          loc.world_pos[1], loc.world_pos[2], loc.size or "Small",
          loc.entrance.x, loc.entrance.y, loc.entrance.tile
        )
    end
  end
end

function loader.load_and_init_mod(mod_folder)
  local manifest = load_manifest(mod_folder)
  apply_manifest(manifest)

  local mod_key = 'mods.' .. mod_folder .. ".init"
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
    log.error("compiling mod '" .. mod_folder .. "': " .. tostring(err))
    return
  end

  local mod_env = sandbox.create_env(mod_folder, manifest)
  setfenv(mod_init_fn, mod_env)

  -- exec mod
  local success, run_err = pcall(mod_init_fn)
  if not success then
    log.error("running mod '" .. mod_folder .. "': " .. tostring(run_err))
  end

  return manifest
end

function loader.reload_mods()
  log.info("Reloading mods...")

  rendering.clear()

  for _, mod_folder in ipairs(reloadable_mods) do
    local target_prefix = "mods." .. mod_folder

    events.clear_for_mod(mod_folder)
    objects.clear_for_mod(mod_folder)
    state.clear_for_mod(mod_folder)

    ffi.C.ck_registry_destroy_objects_for_mod(mod_folder)

    for mod_name in pairs(package.loaded) do
      if mod_name:match("^" .. target_prefix) then
        package.loaded[mod_name] = nil
        log.info("[CK Loader] Unloaded: " .. mod_name)
      end
    end
  end

  for _, mod_folder in ipairs(reloadable_mods) do
    log.info("[CK Loader] Reloading: " .. mod_folder)
    manifest = loader.load_and_init_mod(mod_folder)

    events.emit_for_mod(manifest.id, "onMapEnter")
    events.emit_for_mod(manifest.id, "onModReload")
  end

  log.info("Reload complete!")
end

_G["ckReloadMods"] = loader.reload_mods

return loader
