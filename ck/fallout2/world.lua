-- ck/fallout2/world.lua

local world = {}

function world.spawnCritter(pid)
  return ckSpawnCritter(pid)
end

return world
