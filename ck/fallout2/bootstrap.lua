-- ck/bootstrap.lua

print("[CK] Bootstrapping Construction Kit...")

-- core systems
require('ck.fallout2.events')
require('ck.fallout2.config')

-- mod loader
require('ck.fallout2.loader')

-- boot active mods
ckInitializeMods()

print("[CK] Bootstrap complete!")
