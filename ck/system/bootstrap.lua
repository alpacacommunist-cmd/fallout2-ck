-- bootstrap.lua
package.path = package.path .. ";../?.lua;../?/init.lua"

local bootstrap = {}

require('ck.system.ffi_api')
local ffi = require("ffi")

ck = { active_mods = {} }
ck.log = require('ck.system.log')

local log    = ck.log.new("CK Bootstrap")
local loader = require('ck.system.loader')
local utils  = require('ck.system.utils')

-- Registries, prepares mod namespaces ahead
-- (to be able to safly use tables without warrying about nil)
local objects = require('ck.fallout2.objects')
local timers  = require('ck.fallout2.timers')

local function prepare_mod_registries(mod_id)
  objects.registry[mod_id] = {}
  timers.registry[mod_id]  = {}
  timers.categories.live[mod_id] = {}
end

local function clear_mod_from_registries(mod_id)
  objects.registry[mod_id] = nil
  timers.registry[mod_id] = nil
  timers.categories.live[mod_id] = nil
end

function bootstrap.bootstrap()
  log.info("Bootstrapping active mods...")

  local success_load, active_mods = pcall(require, "mods")

  if not success_load or type(active_mods) ~= "table" then
    log.error("Failed to load mods.lua config! Please ensure gamedir/mods.lua exists and returns a table.")
    active_mods = {}
  end

  for _, mod_id in ipairs(active_mods) do
    prepare_mod_registries(mod_id)
    local success = ffi.C.ck_dispatcher_load_mod(mod_id)

    if success then table.insert(ck.active_mods, mod_id) end

    if not success then
      log.error(string.format("Failed to bootstrap mod '%s' in dispatcher", mod_id))
      clear_mod_from_registries(mod_id)
    end
  end

  log.info("Bootstrap complete! All mods loaded safely.")
end

return bootstrap
