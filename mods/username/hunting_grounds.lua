-- mods/username/hunting_grounds.lua

local world = require('fallout2.world')
local constants = require('fallout2.constants')
local log = require('fallout2.log')

local huntingGrounds = {}
local spawnTable = {
  {
    pid  = constants.pids.GIANT_ANT,
    name = "Giant Ant"
  }
}

function huntingGrounds.isMap(mapId)
  return mapId == constants.maps.HUNTING_GROUNDS
end

function huntingGrounds.spawnCreatures()
  log.print("The hunting grounds feel alive...")

  for _, critter in ipairs(spawnTable) do
    local spawned = world.spawnCritter(critter.pid)

    if spawned then
      log.print("Spawned: " .. critter.name)
    end
  end
end

return huntingGrounds
