-- bootstrap.lua
package.path = package.path .. ";../?.lua;../?/init.lua"

local ffi = require("ffi")

ffi.cdef[[
  bool ck_dispatcher_load_mod(const char* mod_id);
]]

ck.log = require('ck.system.log')

local log    = ck.log.new("CK Bootstrap")
local loader = require('ck.system.loader')

_G["ckBootstrapMods"] = function()
  log.info("Bootstrapping active mods...")

  local active_mods = {
    "game_time_extender",
    "arroyo_expanded",
    "temple_of_trials",
    "natural_growth"
  }

  for _, mod_id in ipairs(active_mods) do
    local success = ffi.C.ck_dispatcher_load_mod(mod_id)

    if not success then
      log.error(string.format("Failed to bootstrap mod '%s' in dispatcher", mod_id))
    end
  end

  log.info("Bootstrap complete! All mods loaded safely.")
end
