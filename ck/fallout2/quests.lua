-- ck/fallout2/quests.lua
local state = require('ck.fallout2.state')
local log   = ck.log.new('CK Quests')

local quests = {}

quests.status = {
  NOT_STARTED = 0,
  ACTIVE      = 1,
  COMPLETED   = 2,
  FAILED      = 3
}

quests.definitions = {}

function quests.register_internal(mod_id, quest_id, config)
  quests.definitions[mod_id] = quests.definitions[mod_id] or {}
  quests.definitions[mod_id][quest_id] = config
end

-- is overriden in sandbox env
function quests.set_internal(mod_id, quest_id, status_value)
  state.set_global(mod_id, "quests", quest_id, status_value)
end

-- is overriden in sandbox env
function quests.get_internal(mod_id, quest_id)
  local value = state.get_global(mod_id, "quests", quest_id)

  return value or quests.status.NOT_STARTED
end

return quests
