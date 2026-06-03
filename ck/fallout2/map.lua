-- ck/fallout2/map.lua

local map = {}

function map.spawnCritter(pid)
  return ckSpawnCritter(pid)
end

function map.getId()
  return ck.map.get_id()
end

return map
