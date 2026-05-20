-- mods/username/test.lua

print("hello from lua to linux terminal (loaded early)")

local log = require('fallout2.log')
local events = require('fallout2.events')

events.on('onGameStart', function()
    log.print("STATION CK: ONLINE AND READY TO MOD!")
end)

events.on('onGameStart', function()
    log.print("You see: A Giant Ant! Oh wait, actually it's LuaJit!")
end)

