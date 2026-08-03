local state = require('ck.fallout2.state')
local assets = require('ck.fallout2.assets')

local knowledge = {}
local log       = ck.log.new('events.lua')
local utils     = require('ck.system.utils')

knowledge.registry   = {}
knowledge.dude_state = {}

function knowledge.register(config)
  assert(config.id,   "Unique knowledge ID is required!")
  assert(config.name, "Knowledge name is required!")

  knowledge.registry[config.id] = {
    name        = config.name,
    frm_id      = config.frm and assets.resolve(config.frm) or 72,
    max_rank    = config.max_rank or 1,
    ranks       = config.ranks or {},
    next_level  = config.next_level or function(rank) return 50 end
  }

  return config.id
end

function knowledge.grant(id)
  if not knowledge.dude_state[id] then
    knowledge.dude_state[id] = { rank = 1, progress = 0 }
    log.debug("Dude learned new knowledge: %s", knowledge.registry[id].name)

    utils.print_table(knowledge.dude_state, log)
  end
end

return knowledge
