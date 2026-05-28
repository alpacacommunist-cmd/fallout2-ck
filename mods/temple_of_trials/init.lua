-- mods/temple_of_trials/init.lua

print("[Mod] Loading Temple of Trials...")

local events = require('fallout2.events')
local map    = require('fallout2.map')

local outskirts = require('temple_of_trials.outskirts')

events.on('onMapEnter', function()
    local mapId = map.getId()

    if mapId ~= 126 then
        return
    end

    ck.rendering.draw_scenery(704, -500, -360)

    print("[Temple] Temple of Trials loaded.")
    print("[Temple] scenery count: " .. tostring(#outskirts.scenery))
end)
