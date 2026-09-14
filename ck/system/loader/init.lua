-- ck/system/loader/init.lua
local ffi = require("ffi")

local sandbox   = require('ck.system.loader.sandbox')

local rendering = require('ck.fallout2.rendering')
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

-- function loader.parse_manifest(mod_id)
--   local key = 'mods.' .. mod_id .. '.mod'
--
--   local ok, manifest = pcall(require, key)
--
--   if not ok or type(manifest) ~= 'table' then
--     log.warn("WARNING: no manifest for " .. mod_id)
--     return nil
--   end
--
--   return manifest
-- end

local function apply_manifest(manifest)
  if not manifest then return end

  if manifest.assets then
  end

  if manifest.locale then
    i18n.register(manifest.id, manifest.locale)
  end
end

function loader.exec_mod(mod_data)
  local manifest = mod_data.manifest
  local mod_id   = manifest.id
  apply_manifest(manifest)

  local is_library = manifest.type == "library"
  local mod_key = 'mods.' .. mod_id .. ".init"
  local file_path = "../" .. mod_key:gsub("%.", "/") .. ".lua"

  local file = io.open(file_path, "r")
  if not file then
    log.error("Cannot open mod file: " .. file_path)
    return false
  end
  local content = file:read("*a")
  file:close()

  -- compile file into function
  local mod_init_fn, err = loadstring(content, "@" .. file_path)
  if not mod_init_fn then
    log.error("compiling mod '" .. mod_id .. "': " .. tostring(err))
    return false
  end

  local mod_env = sandbox.create_env(mod_id, manifest)
  setfenv(mod_init_fn, mod_env)

  if is_library then
    local success, result = pcall(mod_init_fn)

    if not success then
      log.error("running library '" .. mod_id .. "': " .. tostring(result))
      return false
    end

    if type(result) == "table" then
      ck.loaded_libs[mod_id] = result
      log.info(string.format("Library '%s' registered to ck.libs.%s", mod_id, mod_id))
    else
      log.warn(string.format("Library '%s' loaded but did not return an API table!", mod_id))
    end
  else
    registries.init_mod(mod_id)
    ffi.C.ck_dispatcher_add_mod(mod_id)

    local success = mod_tools.exec_with_mod_context(mod_id, mod_init_fn)

    if not success then
      registries.clear_mod(mod_id)
      ffi.C.ck_dispatcher_remove_mod(mod_id)

      return false
    end
  end

  return true
end

function loader.reload_mods()
  for _, mod_id in ipairs(reloadable_mods) do
    log.header("Reloading mod: %s", mod_id)
    log.info("Clearing out resources for: %s", mod_id)

    ffi.C.ck_critter_reset_prototypes_for_mod(mod_id)
    ffi.C.ck_registry_clear_for_mod(mod_id)
    ffi.C.ck_map_clear_camera_borders_for_mod(mod_id)
    -- TODO: remove from reload sequence
    -- ffi.C.ck_config_clear_mod_patches(mod_id)

    -- clear lua registries
    registries.clear_mod(mod_id)

    local target_prefix = "mods." .. mod_id

    for mod_name in pairs(package.loaded) do
      if mod_name:match("^" .. target_prefix) then
        package.loaded[mod_name] = nil
        log.info("Unloaded: " .. mod_name)
      end
    end

    local success = ffi.C.ck_dispatcher_load_mod(mod_id)

    if success then
      ffi.C.ck_dispatcher_emit_for_mod(mod_id, "map_enter")
      ffi.C.ck_dispatcher_emit_for_mod(mod_id, "onModReload")
    end
  end

  log.info("Reload complete!")
end

return loader
