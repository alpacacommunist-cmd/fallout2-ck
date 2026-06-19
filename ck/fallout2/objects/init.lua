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

events.on('onMapEnter', function()
  print("[CK Objects] Map Enter signaled! Cleared Lua objects registry.")
  objects.registry = {}
end)

return objects
