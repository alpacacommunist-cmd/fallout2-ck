-- mods/temple_of_trials/init.lua

print("[Mod] Loading Temple of Trials...")

local events    = require('fallout2.events')
local map       = require('fallout2.map')
local rendering = require('fallout2.rendering')

local outskirts = require('temple_of_trials.outskirts')

events.on('onMapEnter', function()
    local mapId = map.getId()

    if mapId ~= 126 then
        return
    end

    for _, scenery in ipairs(outskirts.scenery) do
      rendering.add_scenery(scenery.fid, outskirts.anchorTile, scenery.x, scenery.y)
    end

    for _, tile in ipairs(outskirts.tiles) do
      -- rendering.add_tile(tile.fid, outskirts.anchorTile, tile.x, tile.y)
    end

    print("[Temple] Temple of Trials loaded.")
    print("[Temple] scenery count: " .. tostring(#outskirts.scenery))
end)
