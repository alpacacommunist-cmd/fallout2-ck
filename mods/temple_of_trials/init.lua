-- mods/temple_of_trials/init.lua

print("[Mod] Loading Temple of Trials...")

local events    = require('ck.fallout2.events')
local map       = require('ck.fallout2.map')
local assets = require('ck.fallout2.assets')

local outskirts = require('mods.temple_of_trials.outskirts')

events.on('onModReload', function()
  map.renderingRefresh()
end)

events.on('onMapEnter', function()
  if map.getId() ~= 126 then
    return
  end

  map.setBorders(95, 113, 82, 110)

  for _, scenery in ipairs(outskirts.mountainScenery) do
    map.addScenery(scenery.fid, scenery.tile)
  end

  for _, tile in ipairs(outskirts.mountainTiles) do
    map.addTile(tile.fid, tile.tile)
  end

  for _, tile in ipairs(outskirts.removeBlockers) do
    map.removeBlocker(tile)
  end

  for _, tile in ipairs(outskirts.createBlockers) do
    map.createBlocker(tile)
  end

  -- map.tools.spawnBrush(15290, 8, 0.7, {263, 264})
  map.tools.spawnBrush(19472, 2, 0.3, {956, 957, 958})
  -- map.tools.spawnBrush(18313, 2, 0.3, {956, 957, 958})
  map.tools.spawnBrush(19472, 2, 0.3, {"temple_of_trials:tiles/grass01", "temple_of_trials:tiles/grass02"})


  local forestLeft = {
    "M MMM  MM  M",
    "  MM   MMM  ",
    "   mm  mm   ",
    "GGG      GG "
  }

  map.tools.spawnMask(17516, forestLeft, {
    M = { assets = {"temple_of_trials:scenery/tree10"}, type = "scenery", block = true },
    m = { assets = {"temple_of_trials:scenery/tree11"}, type = "scenery", block = true },
    G = { assets = {"temple_of_trials:tiles/grass01"}, type = "tile", block = false }
  })

  asset = assets.resolve("temple_of_trials:scenery/tree10")
  map.registerObject(asset.artId, 19094)

  print("[Temple] Temple of Trials loaded.")
  -- print("[Temple] scenery count: " .. tostring(#outskirts.scenery))
end)
