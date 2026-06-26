-- ck/bootstrap.lua

package.path = package.path .. ";../?.lua;../?/init.lua"

ck.log    = require('ck.fallout2.log')
local log = ck.log.new("CK Bootstrap")

-- mod IDS
local active_mods = {
  "game_time_extender",
  "arroyo_expanded",
  "temple_of_trials"
}

log.info("Bootstrapping Construction Kit...")

-- mod loader
local loader = require('ck.fallout2.loader')

for _, mod_folder in ipairs(active_mods) do loader.load_and_init_mod(mod_folder) end

log.info("Bootstrap complete!")
