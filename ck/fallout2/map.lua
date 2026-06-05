-- ck/fallout2/map.lua
--

local map = {}

function map.spawnCritter(pid)
  return ckSpawnCritter(pid)
end

function map.getId()
  return ck.map.get_id()
end

function map.addScenery(fid, tile)
  ck.map.add_scenery(fid, tile)
end

function map.addTile(fid, tile)
  ck.map.add_tile(fid, tile)
end

function map.setBorders(left, right, top, bottom)
  ck.map.set_camera_borders(left, right, top, bottom)
end


return map
