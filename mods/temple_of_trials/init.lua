-- mods/temple_of_trials/init.lua

local map       = require('ck.fallout2.map')
local assets    = require('ck.fallout2.assets')
local critters  = require('ck.fallout2.objects.critters')
local state     = require('ck.fallout2.state')
local behaviors = require('ck.fallout2.objects.critters.behaviors')
local locations = require('ck.fallout2.locations')

local outskirts = require('.outskirts')

-- local new_hunting_grounds = locations.override_map(35,{
--   map_file = 'tstcv', name = "Hunting Grounds", sub_name = "", music = "07desert"
-- })
--
-- log.info("new_hunting_grounds id: " .. new_hunting_grounds)

-- local new_map_id = locations.register_map({
--     map_file = 'tstcv',
--     name     = "Secret Hunting Grounds",
--     music    = "07desert"
-- })
--
-- locations.expand(0, {
--     lookup_name = "Secret Hunting Grounds",
--     townmap_x   = 150,
--     townmap_y   = 220
-- })

new_location_id  = locations.register({ name = "Test Caves", world_x = 220, world_y = 140, size = "small" })
local test_caves = locations.register_map({
    map_file = 'tstcv',
    name     = "Secret Caves",
    sub_name = "Very secret",
    music    = "07desert"
})

locations.expand(new_location_id, { lookup_name = "Secret Caves" })


events.on('onModReload', function()
  map.rendering_refresh()
end)

events.on('onMapEnter', function()
  local map_id = map.get_id()

  if map_id ~= 4 then return end

-- [CK DBG] [INFO] SELECTED tile=21948 (51, 109)
-- [CK DBG] [INFO] SELECTED tile=22152 (47, 110)
-- [CK DBG] [INFO] SELECTED tile=24153 (46, 120)
-- [CK DBG] [INFO] SELECTED tile=24359 (40, 121)
-- entrance_3=Off,100,150,Arroyo Wilderness,-1,-1,0

  map.exit_grid.spawn_in_rect(21948, 22152, 24153, 24359, {
    map = new_map_id,
    tile = -1,
    elevation = 0,
    rotation = 1,
    style = 2
  })


  local ralph = critters.register("ralph_arroyo", 16777217, 19905, {
    name        = 'Ralph',
    description = 'Ralph the Wanderer'
  })

  ralph.stats = { max_hp = 10, hp = 1 }

  ralph:set_behavior(behaviors.wander, 3)
  state.track(ralph, { save_interval_seconds = 5 })
end)

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

  local asset = assets.resolve('temple_of_trials:scenery/tree10')

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
end)
