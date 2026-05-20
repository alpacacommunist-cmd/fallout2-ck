-- mods/game_time_extender/init.lua
print("[Mod] Loading Game Time Extender...")

local config = require('fallout2.config')
local log    = require('fallout2.log')
local events = require('fallout2.events')

-- we're gona use a separate config module
-- engine doesn't do none of this yet, that's just front
config.set('max_game_time_years', 100)


-- Check if monitor still works. this actually works
events.on('onGameStart', function()
    log.print("You see: A Giant Ant! Oh wait, actually it's LuaJit!")
end)
