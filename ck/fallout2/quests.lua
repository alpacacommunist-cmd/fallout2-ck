-- ck/fallout2/quests.lua

local ck = require('ck')
local ffi = require('ffi')


local state = require('ck.fallout2.state')
local log   = ck.log.new('quests.lua')

local quests = {}

quests.status = {
  NOT_STARTED = 0,
  ACTIVE      = 1,
  COMPLETED   = 2,
  FAILED      = 3
}

quests.definitions = {}

function quests.register(quest_id, config)
  local mod_id = ck.tools.current_mod_id()

  quests.definitions[mod_id] = quests.definitions[mod_id] or {}
  quests.definitions[mod_id][quest_id] = config
end

function quests.set(quest_id, status_value)
  local mod_id = ck.tools.current_mod_id()

  if not state.db.global[mod_id] then state.db.global[mod_id] = {} end
  if not state.db.global[mod_id].quests then state.db.global[mod_id].quests = {} end

  state.db.global[mod_id].quests[quest_id] = status_value
  -- state.set_global(mod_id, "quests", quest_id, status_value)
end

function quests.get(quest_id)
  local mod_id = ck.tools.current_mod_id()

  if not state.db.global[mod_id] then state.db.global[mod_id] = {} end
  if not state.db.global[mod_id].quests then state.db.global[mod_id].quests = {} end

  local value = state.db.global[mod_id].quests[quest_id]

  return value or quests.status.NOT_STARTED
end

return quests
