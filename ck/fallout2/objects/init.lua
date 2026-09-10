local registries = require('ck.system.registries')

local objects = {
  registry = registries.objects,

  PROC_NAMES = {
    [3]  = "description",
    [4]  = "pickup",
    [6]  = "use",
    [11] = "talk",
    -- [12] = "critter", -- doesn't work bc gScriptsLists, using own on_map_update from ck_script.cc
    [13] = "combat",
    [14] = "damage",
    [18] = "destroy",
    [21] = "look_at",
    [22] = "timed",
    -- [23] = "map_update", -- doesn't work bc gScriptsLists, using own on_map_update from ck_script.cc
    [24] = "push"
  },

  TYPES = {
    [0] = 'item', [1] = 'critter', [2] = 'scenery',
    [3] = 'wall', [4] = 'tile', [5] = 'misc', [6] = 'interface', [7] = 'inventory',
    [8] = 'head', [9] = 'background'
  }
}

local log = ck.log.new('objects/init.lua')

return objects
