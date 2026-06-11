-- ck/bootstrap.lua

print("[CK] Bootstrapping Construction Kit...")

package.path = package.path .. ";../?.lua;../?/init.lua"

-- core systems
require('ck.fallout2.events')
require('ck.fallout2.config')

-- mod loader
local loader = require('ck.fallout2.loader')

-- boot active mods
loader.initialize()

print("[CK] Bootstrap complete!")
