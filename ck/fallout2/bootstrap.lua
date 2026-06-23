-- ck/bootstrap.lua

print("[CK] Bootstrapping Construction Kit...")

package.path = package.path .. ";../?.lua;../?/init.lua"

ck.log    = require('ck.fallout2.log')
local log = ck.log.new("CK Bootstrap")

-- core systems
require('ck.fallout2.events')
require('ck.fallout2.config')

-- mod loader
local loader = require('ck.fallout2.loader')

-- boot active mods
loader.initialize()

log.info("Bootstrap complete!")
