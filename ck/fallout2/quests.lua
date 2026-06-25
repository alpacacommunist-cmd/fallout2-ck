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

function quests.register(quest_id, config)
  local mod_id = events.current_active_mod or "unknown"

  quests.definitions[mod_id] = quests.definitions[mod_id] or {}
  quests.definitions[mod_id][quest_id] = config
end

function quests.set(quest_id, status_value)
  local mod_id = events.current_active_mod or "unknown"

  state.set_global(mod_id, "quests", quest_id, status_value)
end

function quests.get(quest_id)
  local mod_id = events.current_active_mod or "unknown"
  local value = state.get_global(mod_id, "quests", quest_id)

  return value or quests.status.NOT_STARTED
end

return quests
