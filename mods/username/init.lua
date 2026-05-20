-- mods/username/init.lua
print("[Mod] Loading Username Test Mod...")

local events = require('fallout2.events')
local log    = require('fallout2.log')

events.on('onGameStart', function()
    log.print("You see: A Giant Ant! Nope! Still LuaJit!")
end)
