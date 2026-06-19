local events = require('ck.fallout2.events')

local objects = {
  registry = {},

  PROC_NAMES = {
    [3]  = "description",
    [4]  = "pickup",
    [6]  = "use",
    [11] = "talk",
    [12] = "critter",
    [13] = "combat",
    [14] = "damage",
    [21] = "look_at",
    [22] = "timed",
    [23] = "map_update",
    [24] = "push"
  }
}

function ckOnObjectsDestroyed()
  objects.registry = {}
  print("[CK Objects] Registry cleared")
end

return objects
