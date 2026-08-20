-- mods/temple_of_trials/init.lua
local map       = require('ck.fallout2.map')
local assets    = require('ck.fallout2.assets')
local critters  = require('ck.fallout2.objects.critters')
local state     = require('ck.fallout2.state')
local behaviors = require('ck.fallout2.objects.critters.behaviors')
local locations = require('ck.fallout2.locations')
local player    = require('ck.fallout2.player')
local stats     = require('ck.fallout2.objects.critters.stats')
local items     = require('ck.fallout2.objects.items')

local outskirts = require('.outskirts')
local utils     = require('ck.system.utils')
--

map.register_borders(126, {left = 95, right = 113, top = 82, bottom = 110})

events.on('onModReload', function()
  map.rendering_refresh()
end)

events.on('onGameLoaded', function()
  log.info(player.stats.strength)
  log.info(player.stats.intelligence)
  log.info(player.gender)
  log.info(player.level)

  log.info('small_guns: ' .. tostring(player.skills.small_guns))

  -- player.set_base_stat('perception', 8)
  -- player.set_base_stat('strength', 8)
  -- player.set_base_stat('endurance', 7)
end)

events.on('onMapEnter', function(map_id)
  if map_id ~= 4 then return end

  local ralph = critters.register("ralph_arroyo", 16777217, 19905, {
    name        = 'Ralph',
    description = 'Ralph the Wanderer'
  })

  ralph.stats = { max_hp = 10, hp = 1 }

  ralph:set_behavior(behaviors.wander, 3)
  state.track(ralph, { save_interval_seconds = 5 })
end)

events.on('onMapEnter', function(map_id)
  if map_id ~= 72 then return end

  -- local ralph = critters.register("ralph_the_second_arroyo", 16777217, 25103, {
  local ralph = critters.register("ralph_the_second_arroyo", 16777217, 18536, {
    name        = 'Ralph The Second',
    description = 'Ralph the Wanderer',
    elevation = 1
  })

  ralph.stats = { max_hp = 10, hp = 1 }

  ralph:set_behavior(behaviors.wander, 3)
  state.track(ralph, { save_interval_seconds = 5 })

  if (not ralph.is_dead) then
    ralph:give_item(items.PID_KNIFE, 1)
    ralph:give_item(items.PID_STIMPAK, 5)
  end

  if (map.elevation() == 1) then
    map.tools.objects.brush(19907, 2, 0.3, {33555377, 33555378, 33555379})
  end
end)

events.on('onMapEnter', function()
  if map.get_id() ~= 126 then return end

  -- local db = require('ck.fallout2.db')
  --
  -- local proto = db.get_by_pid(16777221)
  -- if proto then
  --   log.info("proto name" .. proto.name)
  --   log.info("proto filename" .. proto.filename)
  --   log.info("description: " .. proto.description)
  --   log.info("SID: " .. proto.sid)
  -- end

  -- map.exit_grid.destroy_in_rect(21514, 22727, 24113, 24323);
  -- --
  -- map.exit_grid.spawn_in_line(22533, 24734, {
  --   map = 4, tile = 11683, elevation = 0, rotation = 1, style = 4
  -- },3 )

  -- local found_objects = map.find_at_tile(21101)
  -- local klint = map.find_at_tile(21101):find_by_pid(16777219)

  local found_objects = map.find_by_pid(16777219)
  local klint_obj = found_objects[1]
  log.warn(klint_obj.pid)
  log.warn(klint_obj:is_critter())
  log.warn("lua id: " .. tostring(klint_obj.lua_id))

  local klint = klint_obj:bind()

  klint:on('talk', function(self)
    self:float_message('Lua intercepted my script, Chosen One. My ID is: ' .. tostring(self:id()), 1)
  end)
  klint:on('push', function(self)
    self:float_message('denied', 1)
    return true
  end)

  -- local orig_klint_sid = klint:restore()
  -- log.warn(orig_klint_sid)
  -- klint:on('push', function(self)
  --   self:float_message('Denied', 2)
  -- end)

  local asset = assets.resolve('temple_of_trials:scenery/tree10')

  map.batch.scenery(outskirts.mountain_scenery)
  map.batch.tiles(outskirts.mountain_tiles)

  map.batch.clear(outskirts.remove_blockers)
  map.batch.blockers(outskirts.create_blockers)

  map.tools.render.floor.brush(19472, 2, 0.3, {"temple_of_trials:tiles/grass01", "temple_of_trials:tiles/grass02"},
    { type = "tile" })
  map.tools.objects.floor.brush(19472, 2, 0.3, {33555377, 33555378, 33555379})

  map.tools.render.floor.mask(17516, {
    "M MMM  MM  M",
    "  MM   MMM  ",
    "   mm  mm   ",
    "GGG      GG "
  }, {
    M = { assets = {"temple_of_trials:scenery/tree10"}, block = false },
    m = { assets = {"temple_of_trials:scenery/tree11"}, block = false },
    G = { assets = {"temple_of_trials:tiles/GRASS01"}, type = "tile" }
  })

  map.render.roof.tile(0x040005E5, 18484, 1)
  map.render.roof.tile(0x040005E5, 18684, 1)
  map.render.roof.tile(0x040005E5, 18685, 1)
  map.render.roof.tile(0x040005E5, 18884, 1)
  map.render.roof.tile(0x040005E5, 18885, 1)
  map.render.roof.tile(0x040005E5, 18886, 1)
  map.render.roof.tile(0x040005E5, 19085, 1)
  map.render.roof.tile(0x040005E5, 19086, 1)
  map.render.roof.tile(0x040005E5, 19286, 1)
  map.render.roof.tile(0x040005E5, 19287, 1)
  map.render.roof.tile(0x040005E5, 19288, 1)
  map.render.roof.tile(0x040005E5, 19487, 1)

  -- map.tools.render.roof.tile(0x040005E5, 13893)
  -- map.tools.render.roof.tile(0x040005E5, 14896)
  -- map.tools.render.roof.tile(0x040005E5, 15498)
  -- map.tools.render.roof.tile(0x040005E5, 15098)
  -- map.tools.render.roof.tile(0x040005E5, 15899)
  -- map.tools.render.roof.tile(0x040005E5, 12890)

  -- map.place("temple_of_trials:scenery/tree10", 19094)
end)
