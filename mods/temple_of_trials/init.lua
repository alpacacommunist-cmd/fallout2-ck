-- mods/temple_of_trials/init.lua

print("[Mod] Loading Temple of Trials...")

local events    = require('fallout2.events')
local map       = require('fallout2.map')
local rendering = require('fallout2.rendering')

local outskirts = require('temple_of_trials.outskirts')

events.on('onMapEnter', function()
  if map.getId() ~= 126 then
    return
  end

  rendering.setBorders(109, 114, 88, 98)

  for _, scenery in ipairs(outskirts.mountainScenery) do
    rendering.addScenery(scenery.fid, outskirts.anchorTile, scenery.x, scenery.y)
  end

  for _, tile in ipairs(outskirts.mountainTiles) do
    rendering.addTile(tile.fid, outskirts.anchorTile, tile.x, tile.y)
  end

  print("[Temple] Temple of Trials loaded.")
  -- print("[Temple] scenery count: " .. tostring(#outskirts.scenery))
end)
