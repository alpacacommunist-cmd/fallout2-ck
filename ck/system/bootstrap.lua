-- bootstrap.lua
package.path = package.path .. ";../?.lua;../?/init.lua"

ck.log = require('ck.system.log')

local loader = require('ck.system.loader')
local log    = ck.log.new("CK Bootstrap")

log.info("Bootstrapping Construction Kit...")

local active_mods = {
  "game_time_extender",
  "arroyo_expanded",
  "temple_of_trials"
}

for _, mod_id in ipairs(active_mods) do
  local ok, err = xpcall(function() loader.load_and_init_mod(mod_id) end, debug.traceback)

  if not ok then
    log.error(string.format("Failed to bootstrap mod '%s':\n%s", mod_id, err))
  end
end

log.info("Bootstrap complete!")
