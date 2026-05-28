-- ck/bootstrap.lua

print("[CK] Bootstrapping Construction Kit...")

-- core systems
require('fallout2.events')
require('fallout2.config')

-- mod loader
require('fallout2.loader')

-- boot active mods
ckInitializeMods()

print("[CK] Bootstrap complete!")

print(ck)
print(ck.rendering)
print(ck.rendering.draw_scenery)
