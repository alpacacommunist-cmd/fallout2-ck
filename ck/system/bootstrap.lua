-- bootstrap.lua
package.path = package.path .. ";../?.lua;../?/init.lua"

require('ck.system.ffi_api')
local ffi = require("ffi")

ck = {}
ck.log = require('ck.system.log')

local log    = ck.log.new("CK Bootstrap")
local loader = require('ck.system.loader')

_G["ckBootstrapMods"] = function()
  log.info("Bootstrapping active mods...")

  local success_load, active_mods = pcall(require, "mods")

  if not success_load or type(active_mods) ~= "table" then
    log.error("Failed to load mods.lua config! Please ensure gamedir/mods.lua exists and returns a table.")
    active_mods = {}
  end

  for _, mod_id in ipairs(active_mods) do
    local success = ffi.C.ck_dispatcher_load_mod(mod_id)

    if not success then
      log.error(string.format("Failed to bootstrap mod '%s' in dispatcher", mod_id))
    end
  end

  log.info("Bootstrap complete! All mods loaded safely.")
end
