local ffi   = require('ffi')
local state = require('ck.fallout2.state')
local assets = require('ck.fallout2.assets')

local knowledge = {}
local log       = ck.log.new('knowledge.lua')

knowledge.registry     = {}
knowledge.player_state = state.db.player.knowledge

function knowledge.sync()
  for id, state in pairs(knowledge.player_state) do
    local static_info = knowledge.registry[id]
    if static_info then
      local full_desc = static_info.ranks[state.rank].desc .. "\n\n[ Progress: " .. state.progress .. "/50 ]"

      fid    = assets.resolve(static_info.frm)
      frm_id = ffi.C.ck_ids_frm_id_from_fid(fid)

      local ffi_data = ffi.new("CkKnowledgeFFI")
      ffi_data.id = id
      ffi_data.name = string.format("%s (%d)", static_info.name, state.rank)
      ffi_data.description = full_desc
      ffi_data.frm_id = frm_id

      ffi.C.ck_knowledge_push_cache(ffi_data)
    end
  end
end

function knowledge.register(config)
  assert(config.id,   "Unique knowledge ID is required!")
  assert(config.name, "Knowledge name is required!")

  knowledge.registry[config.id] = {
    name        = config.name,
    frm         = config.frm,
    max_rank    = config.max_rank or 1,
    ranks       = config.ranks or {},
    next_level  = config.next_level or function(rank) return 50 end
  }

  return config.id
end

function knowledge.grant(id)
  if not knowledge.player_state[id] then
    knowledge.player_state[id] = { rank = 1, progress = 0 }
    log.debug("Player learned new knowledge: %s", knowledge.registry[id].name)
  end
end

return knowledge
