-- bootstrap.lua
package.path = package.path .. ";../?.lua;../?/init.lua"

local bootstrap = {}

require('ck.system.ffi_api')
local ffi = require("ffi")

ck = { active_mods = {}, libs = {} }
ck.log = require('ck.system.log')

local log    = ck.log.new('bootstrap.lua')
local loader = require('ck.system.loader')
local mod_tools = require('ck.system.mod_tools')

function bootstrap.bootstrap()
  log.info("Bootstrapping active mods...")

  -- parse gamedir/mods.lua (list of mods)
  local success_load, active_mods = pcall(require, "mods")
  if not success_load or type(active_mods) ~= "table" then
    log.error("Failed to load mods.lua config! Please ensure gamedir/mods.lua exists and returns a table.")
  end

  local libraries = {}
  local gameplay_mods = {}

  -- parse and analyze manifests
  for _, mod_id in ipairs(active_mods) do
    local manifest = loader.parse_manifest(mod_id)

    if manifest and manifest.type == "library" then
      table.insert(libraries, { id = mod_id, manifest = manifest })
    else
      table.insert(gameplay_mods, { id = mod_id, manifest = manifest })
    end
  end

  for _, mod_data in ipairs(gameplay_mods) do
    loader.exec_mod(mod_data.id, mod_data.manifest)
  end

  log.info("Bootstrap complete! All mods loaded safely.")
end

return bootstrap
