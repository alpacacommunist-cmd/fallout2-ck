-- ck/fallout2/map/init.lua

local geometry = require('fallout2.map.geometry')
local tools    = require('fallout2.map.tools')
local map      = {}

-- apply bindings to `tools`
tools._applyValue = function(value, objType, block, tile, mode)
  mode = mode or "place"

  if mode == "draw" then
    if objType == "tile" then
      ck.map.add_tile(value, tile)
    else
      ck.map.add_scenery(value, tile)
    end
  else
    if type(value) == "number" then
      ck.map.create_object(value, tile) -- actual fid, create fallout-ce object
    else
      ck.map.add_scenery(value, tile)  -- custom asset, just draw for now
    end
  end

  if block then ck.map.create_blocker(tile) end
end

tools._onClear = function(tile)
  ck.map.remove_blocker(tile)
end

-- public API
map.getId      = function() return ck.map.get_id() end
map.setBorders = function(l, r, t, b) ck.map.set_camera_borders(l, r, t, b) end

map.addScenery    = function(fid, tile) ck.map.add_scenery(fid, tile) end
map.addTile       = function(fid, tile) ck.map.add_tile(fid, tile) end
map.createBlocker = function(tile) ck.map.create_blocker(tile) end
map.removeBlocker = function(tile) ck.map.remove_blocker(tile) end
map.createObject  = function(fid, tile) ck.map.create_object(fid, tile) end
map.spawnCritter  = function(pid) return ckSpawnCritter(pid) end

map.geometry = geometry
map.tools    = tools

return map
