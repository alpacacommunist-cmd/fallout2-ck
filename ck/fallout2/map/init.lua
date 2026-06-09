-- ck/fallout2/map/init.lua

local geometry = require('ck.fallout2.map.geometry')
local tools    = require('ck.fallout2.map.tools')
local assets   = require('ck.fallout2.assets')
local map      = {}


-- apply bindings to `tools`
tools._applyValue = function(value, objType, block, tile, mode)
  mode  = mode or "place"
  asset = assets.resolve(value)

  -- tiles
  if objType == "tile" or (asset and asset.isTile) then
    ck.map.add_tile(value, tile)

    return
  end

  if type(value) == "number" then
    -- raw fid
    ck.map.create_object(value, tile)
  else
    -- string key
    if mode == "draw" or not asset or asset.fid == -1 then
      -- no fid or explicit draw —> render
      ck.map.add_scenery(value, tile)
    else
      -- got fid —> engine object
      ck.map.create_object(asset.fid, tile)
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

-- value = int  → create_object (raw fid, "place")
-- value = string + mode="draw"  → add_scenery/add_tile directly
-- value = string + mode="place" → assets.resolve
--     fid found  → create_object (transparency, collision)
--     fid not found → add_scenery/add_tile (render mods .frm)

function map.placeObject(assetKey, tile)
  local asset = assets.resolve(assetKey)

  if asset and asset.fid ~= -1 then
    ck.map.create_object(asset.fid, tile)
  else
    ck.map.add_scenery(assetKey, tile)
  end
end

map.geometry = geometry
map.tools    = tools

return map
