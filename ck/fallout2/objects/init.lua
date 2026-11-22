local ck = require('ck')

local objects = {
  registry = ck.registries.objects,

  TYPES = {
    [0] = 'item', [1] = 'critter', [2] = 'scenery',
    [3] = 'wall', [4] = 'tile', [5] = 'misc', [6] = 'interface', [7] = 'inventory',
    [8] = 'head', [9] = 'background'
  }
}

local log = ck.log.new('objects/init.lua')

return objects
