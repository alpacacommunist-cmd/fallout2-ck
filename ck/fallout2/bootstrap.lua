-- ck/bootstrap.lua

print("[CK] Bootstrapping Construction Kit...")

package.path = package.path .. ";../?.lua;../?/init.lua"

-- core systems
require('ck.fallout2.events')
require('ck.fallout2.config')

local dialogue = require('ck.fallout2.dialogue')

function ckOnDialogStart(id)
  dialogue.start(id)
end

-- mod loader
local loader = require('ck.fallout2.loader')

-- boot active mods
loader.initialize()

print("[CK] Bootstrap complete!")
