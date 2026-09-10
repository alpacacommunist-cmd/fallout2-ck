-- ck/fallout2/timers.lua
local ffi = require("ffi")
local game_time = require('ck.fallout2.game_time')
local utils = require('ck.system.utils')

local registries = require('ck.system.registries')

local log   = ck.log.new('timers.lua')

local timers = {}

-- Timer types
timers.timer_types = {
  ["one_time"] = "one_time",
  ["periodic"] = "periodic"
}

-- Timers registry
-- { "temple_of_trials" = { "timer_1" = {}, "timer_2" = {} ... } }
timers.registry = registries.timers

-- Timers categories (for quicker polling)
-- (only stores the tags)
-- { ["live"] = { temple_of_trials = {"tag_one", "tag_two"} ... } }
-- (only live for now)
timers.categories = registries.timer_categories

-- mod_id + "_timer_" + count
timers.generate_timer_id = function(mod_id)
  local count = 0
  if timers.registry[mod_id] then
    for _ in pairs(timers.registry[mod_id]) do count = count + 1 end
  end

  return mod_id .. "_timer_" .. count
end

local function exec_timer_callback(mod_id, callback)
  local raw_context = ffi.C.ck_get_current_mod_id()

  -- previous context (could be nullptr)
  local previous_mod_context = nil
  if raw_context ~= nil then
    previous_mod_context = ffi.string(raw_context)
  end

  local function error_handler(err)
    local traceback = debug.traceback(err, 2)
    logger.error("Timer failed in mod [%s]!\nError: %s", mod_id, traceback)
    return err
  end

  ffi.C.ck_set_current_mod_context(mod_id)
  local success, result = xpcall(callback, error_handler)

  -- restore context (could be nullptr)
  ffi.C.ck_set_current_mod_context(previous_mod_context)

  return success
end

-- removes timer from categories (timers.categories) (flat list)
local function remove_from_categories(tag, mod_id)
  local tags_list = timers.categories.live[mod_id]
  if not tags_list or #tags_list == 0 then return end

  for index = #tags_list, 1, -1 do
    if tags_list[index] == tag then
      table.remove(tags_list, index)
      break
    end
  end
end

-- removes timer from registry/categories
timers.remove = function(tag)
  local mod_id = ffi.string(ffi.C.ck_get_current_mod_id())

  timers.registry[mod_id][tag] = nil
  remove_from_categories(tag, mod_id)
end

-- Map context timers, on map exit timers.registry is cleared
-- (last exec times are written to state db in state.sync_save)
timers.register_timer = function(tag, timer_type, ticks, callback, events_list)
  local mod_id = ffi.string(ffi.C.ck_get_current_mod_id())
  local current_time = game_time.get_time()

  ticks = ticks or 0
  local exec_time = current_time + ticks

  timer_type = timers.timer_types[timer_type] or timers.timer_types.one_time

  if not tag or utils.is_blank(tag) then
    tag = timers.generate_timer_id(mod_id)
  end

  log.debug("timer_type: %s, ticks: %d, current_time: %d, mod_id: %s", timer_type, ticks, current_time, mod_id)

  local timer = {
    tag = tag,
    timer_type = timer_type,
    created_at = current_time,
    ticks = ticks,
    callback = callback,
    mod_id = mod_id,
    events = events_list
  }

  -- check existing timer in registry
  if timers.registry[mod_id] and timers.registry[mod_id][tag] then
    remove_from_categories(tag, mod_id)
  end

  -- if one_time and exec now - exec and return
  if timer_type == "one_time" and ticks == 0 then
    return exec_timer_callback(mod_id, callback)
  end

  local state = require('ck.fallout2.state')
  local state_timer = state.db.maps[ck.map_id][mod_id]["timers"][tag]

  -- check if state has timer's last exec/creation time
  -- needed for savegame/loadgame
  -- If timer isn't in registry yet this should mean game just loaded
  -- In this case apply `created_at` (exec/creation time) from db
  if state_timer and state_timer.timer_type == timer.timer_type then
    timer.created_at = state_timer.created_at
  end

  timers.registry[mod_id][tag] = timer

  -- update timer.categories
  if events_list == nil or #events_list == 0 then
    table.insert(timers.categories.live[mod_id], timer.tag)
  end
end

timers.check_live_timers = function(ticks)
  for mod_id, timer_tags_list in pairs(timers.categories.live) do
    -- log.debug("checking %d timers for mod: [%s]", #timer_tags_list, mod_id)

    for index = #timer_tags_list, 1, -1 do
      local timer_tag = timer_tags_list[index]
      local timer     = timers.registry[mod_id][timer_tag]

      log.debug("timer.exec_time: %d, current_time: %d", (timer.created_at + timer.ticks), ticks)
      if ticks >= (timer.created_at + timer.ticks) then
        exec_timer_callback(timer.mod_id, timer.callback)

        if timer.timer_type == "one_time" then
          -- remove from registry
          timers.registry[mod_id][timer_tag] = nil
          -- remove from categories
          table.remove(timer_tags_list, index)
        end

        -- updates exec time (created_at)
        if timer.timer_type == "periodic" then timer.created_at = game_time.get_time() end
      end
    end
  end
end

return timers
