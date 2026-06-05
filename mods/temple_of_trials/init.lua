-- mods/temple_of_trials/init.lua

print("[Mod] Loading Temple of Trials...")

local events    = require('fallout2.events')
local map       = require('fallout2.map')

local outskirts = require('temple_of_trials.outskirts')

events.on('onMapEnter', function()
  if map.getId() ~= 126 then
    return
  end

  map.setBorders(95, 113, 87, 110)

  for _, scenery in ipairs(outskirts.mountainScenery) do
    map.addScenery(scenery.fid, scenery.tile)
  end

  for _, tile in ipairs(outskirts.mountainTiles) do
    map.addTile(tile.fid, tile.tile)
  end

  print("[Temple] Temple of Trials loaded.")
  -- print("[Temple] scenery count: " .. tostring(#outskirts.scenery))
end)
