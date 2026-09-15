-- bootstrap.lua
local bootstrap = { mod_types_hash_table = {} }

-- system registries
local registries = require('ck.system.registries')

-- set global search path
package.path = package.path .. ";../?.lua;../?/init.lua"

-- Common global namespace
ck = {
  -- quick ref data table
  active_mods = {},
  -- flat list of loaded mods
  active_mods_list = {},
  -- mods sorted by type
  active_mods_by_type = {},
  -- list of loaded lib mods
  loaded_libs = {},

  language = 'english',

  log = require('ck.system.log')
}

for _, mod_type in ipairs(registries.mod_types) do
  -- reserve type table in global namespace
  ck.active_mods_by_type[mod_type] = {}

  -- add type to a hash table for faster checks
  bootstrap.mod_types_hash_table[mod_type] = true
end

local ffi = require('ck.system.ffi_api')
local loader = require('ck.system.loader')

local log = ck.log.new('bootstrap.lua')

----
-- Local helper functions
----
local function mod_keys(mod_id)
  local mod_prefix = 'mods.' .. mod_id

  return {
    ["base"]     = mod_prefix,
    ["manifest"] = mod_prefix .. '.mod',
    ["init"]     = mod_prefix .. '.init',
    ["assets"]   = mod_prefix .. '/assets',
    ["maps"]     = mod_prefix .. '/maps',
  }
end

local function parse_manifest(path)
  local ok, manifest = pcall(require, path)

  if not ok or type(manifest) ~= 'table' then
    log.warn("WARNING: failed to parse manifest: " .. path)
    return nil
  end

  return manifest
end

----
-- Module code
----
function bootstrap.set_language(language)
  ck.language = language
  log.header("Language set to [%s]", ck.language)
end

function bootstrap.bootstrap()
  log.info("Bootstrapping active mods...")

  -- parse gamedir/mods.lua (list of mods)
  local success_load, active_mods = pcall(require, "mods")
  if not success_load or type(active_mods) ~= "table" then
    log.error("Failed to load mods.lua config! Please ensure gamedir/mods.lua exists and returns a table.")
  end

  -- parse and analyze manifests
  for _, mod_id in ipairs(active_mods) do
    local mod_keys = mod_keys(mod_id)
    local manifest = parse_manifest(mod_keys.manifest)

    -- ✨Validating manifest
    if not manifest then
      log.warn("Mod '%s' is missing a valid manifest (mod.lua). Using generic fallback.", mod_id)
      manifest = { id = mod_id, name = "Unnamed Mod (" .. mod_id .. ")", type = registries.default_mod_type }
    end

    -- ensure core attributes
    manifest.type = (bootstrap.mod_types_hash_table[manifest.type] and manifest.type) or registries.default_mod_type
    manifest.id   = manifest.id or mod_id
    manifest.name = manifest.name or manifest.id

    local mod_data = { id = manifest.id, manifest = manifest, keys = mod_keys }

    -- update type categories
    table.insert(ck.active_mods_by_type[manifest.type], mod_data)
    -- update flat list
    if manifest.type == 'gameplay' then
      table.insert(ck.active_mods_list, manifest.id)
    end
    -- update quick ref data table
    ck.active_mods[manifest.id] = mod_data
  end

  for index, mod_type in ipairs(registries.mod_load_sequence_by_type) do
    local mods_by_type = ck.active_mods_by_type[mod_type]

    if mods_by_type and #mods_by_type > 0 then
      log.header("Loading sequence #[%d]: %s", index, mod_type)

      for _, mod_data in ipairs(ck.active_mods_by_type[mod_type]) do
        log.info("Loading mod_id: [%s]", mod_data.id)
        loader.exec_mod(mod_data)
      end
    end
  end

  log.info("Bootstrap complete! All mods loaded safely.")
end

return bootstrap
