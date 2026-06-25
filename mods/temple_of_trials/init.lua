-- mods/temple_of_trials/init.lua

log.info("Loading Temple of Trials...")

local map       = require('ck.fallout2.map')
local assets    = require('ck.fallout2.assets')
local critters    = require('ck.fallout2.objects.critters')
local behaviors    = require('ck.fallout2.behaviors')

local outskirts = require('.outskirts')

events.on('onModReload', function()
  map.rendering_refresh()
end)

-- events.on('onMapEnter', function()
--   local map_id = map.get_id()
--
--   if map_id ~= 4 then return end
--
--   local ralph = critters.register("ralph_arroyo", 16777217, 19905, {
--     name        = 'Ralph',
--     description = 'Ralph the Wanderer'
--   })
--   ralph:set_behavior(behaviors.wander, 3)
--   state.track(ralph, { save_interval_seconds = 5 })
-- end)

events.on('onMapEnter', function()
  if map.get_id() ~= 126 then return end

  map.set_borders(95, 113, 82, 110)

  map.exit_grid.destroy_in_rect(21514, 22727, 24113, 24323);

  map.exit_grid.spawn_in_rect(22132, 23531, 23938, 24731, {
    map = 4,
    tile = 11683,
    elevation = 0,
    rotation = 1,
    style = 2
  })

  map.batch.scenery(outskirts.mountain_scenery)
  map.batch.tiles(outskirts.mountain_tiles)

  map.batch.clear(outskirts.remove_blockers)
  map.batch.blockers(outskirts.create_blockers)

  map.tools.spawn_brush(19472, 2, 0.3, {956, 957, 958}, { mode = "place" })
  map.tools.spawn_brush(19472, 2, 0.3, {"temple_of_trials:tiles/grass01", "temple_of_trials:tiles/grass02"},
    { type = "tile" })

  map.tools.spawn_mask(17516, {
    "M MMM  MM  M",
    "  MM   MMM  ",
    "   mm  mm   ",
    "GGG      GG "
  }, {
    M = { assets = {"temple_of_trials:scenery/tree10"}, block = true },
    m = { assets = {"temple_of_trials:scenery/tree11"}, block = true },
    G = { assets = {"temple_of_trials:tiles/GRASS01"}, type = "tile" }
  })

  map.place("temple_of_trials:scenery/tree10", 19094)

  log.info("Temple of Trials loaded.")
end)
