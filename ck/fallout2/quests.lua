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

local function get_caller_mod_id()
  for level = 2, 10 do
    local  success, env = pcall(getfenv, level)
    if not success or not env then break end

    if env.mod_id then return env.mod_id end
  end

  return "unknown"
end

function quests.register(quest_id, config)
  local mod_id = get_caller_mod_id()

  quests.definitions[mod_id] = quests.definitions[mod_id] or {}
  quests.definitions[mod_id][quest_id] = config
end

function quests.set(quest_id, status_value)
  local mod_id = get_caller_mod_id()

  state.set_global(mod_id, "quests", quest_id, status_value)
end

function quests.get(quest_id)
  local mod_id = get_caller_mod_id()
  local value = state.get_global(mod_id, "quests", quest_id)

  return value or quests.status.NOT_STARTED
end

return quests
