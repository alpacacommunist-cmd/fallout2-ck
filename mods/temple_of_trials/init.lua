-- mods/temple_of_trials/init.lua

print("[Mod] Loading Temple of Trials...")

local events    = require('ck.fallout2.events')
local map       = require('ck.fallout2.map')
local assets = require('ck.fallout2.assets')

local outskirts = require('mods.temple_of_trials.outskirts')

events.on('onModReload', function()
  map.rendering_refresh()
end)

events.on('onMapEnter', function()
  if map.get_id() ~= 126 then return end

  map.set_borders(95, 113, 82, 110)

  -- map.remove_all_by_pid(83886100) -- Базовая сетка
  -- map.remove_all_by_pid(83886098) -- Базовая сетка
  -- map.remove_all_by_pid(83886097) -- Базовая сетка

  -- for _, tile in ipairs(outskirts.new_exit_grid) do
  --   map.create_pid_at(83886100, tile)
  -- end

  -- map.create_pid_at(33554481, outskirts.new_exit_marker_tile)
  -- map.create_pid_at(83886100, outskirts.new_exit_marker_tile)
  -- map.create_pid_at(83886098, outskirts.new_exit_marker_tile)
  -- map.create_pid_at(83886097, outskirts.new_exit_marker_tile)

  for _, scenery in ipairs(outskirts.mountainScenery) do
    map.place(scenery.fid, scenery.tile, { mode = "draw" })
  end

  for _, tile in ipairs(outskirts.mountainTiles) do
    map.place(tile.fid, tile.tile, { type = "tile" })
  end

  for _, tile in ipairs(outskirts.removeBlockers) do map.remove_blocker(tile) end
  for _, tile in ipairs(outskirts.createBlockers) do map.create_blocker(tile) end

  map.tools.spawnBrush(19472, 2, 0.3, {956, 957, 958}, { mode = "place" })
  map.tools.spawnBrush(19472, 2, 0.3, {"temple_of_trials:tiles/grass01", "temple_of_trials:tiles/grass02"},
    { type = "tile" })

  map.tools.spawnMask(17516, {
    "M MMM  MM  M",
    "  MM   MMM  ",
    "   mm  mm   ",
    "GGG      GG "
  }, {
    M = { assets = {"temple_of_trials:scenery/tree10"}, block = true },
    m = { assets = {"temple_of_trials:scenery/tree11"}, block = true },
    G = { assets = {"temple_of_trials:tiles/grass01"}, type = "tile" }
  })

  map.place("temple_of_trials:scenery/tree10", 19094)

  print("[Temple] Temple of Trials loaded.")
  -- print("[Temple] scenery count: " .. tostring(#outskirts.scenery))
end)
