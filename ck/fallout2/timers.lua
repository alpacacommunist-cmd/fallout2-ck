-- ck/fallout2/timers.lua
local ffi = require("ffi")
local game_time = require('ck.fallout2.game_time')
local utils = require('ck.system.utils')

local log   = ck.log.new('game_time.lua')

local timers = {}

-- Timer types
timers.timer_types = {
  ["one_time"] = "one_time",
  ["periodic"] = "periodic"
}

-- Timers registry
-- { "temple_of_trials" = { "timer_1" = {}, "timer_2" = {} ... } }
timers.registry = {}

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

  -- restore context
  if previous_mod_context == nil then
    ffi.C.ck_set_current_mod_context(nil)
  else
    ffi.C.ck_set_current_mod_context(previous_mod_context)
  end

  return success
end

timers.register_timer = function(tag, timer_type, ticks, callback, params)
  local mod_id = ffi.string(ffi.C.ck_get_current_mod_id())
  local current_time = game_time.get_time()

  ticks = ticks or 0
  exec_time = current_time + ticks

  timer_type = timers.timer_types[timer_type] or timers.timer_types.one_time

  if not tag or utils.is_blank(tag) then
    tag = timers.generate_timer_id(mod_id)
  end

  log.error("timer_type: %s, ticks: %d, current_time: %d, mod_id: %s", timer_type, ticks, current_time, mod_id)

  -- check existing timer in registry
  if timers.registry[mod_id] and timers.registry[mod_id][tag] then
    local timer = timers.registry[mod_id][tag]
  else
    if timer_type == "one_time" and exec_time <= ticks then
      exec_timer_callback(mod_id, callback)
    else
      -- new timer, write to registry, save to state db
      local state = require('ck.fallout2.state')

      timers.registry[mod_id] = timers.registry[mod_id] or {}
      state.db.timers[mod_id] = state.db.timers[mod_id] or {}

      local timer = {
        tag = tag,
        timer_type = timer_type,
        created_at = current_time,
        ticks = ticks,
        callback = callback,
        mod_id = mod_id,
        params = params
      }

      timers.registry[mod_id][tag] = timer
      state.db.timers[mod_id][tag] = { created_at = current_time }
    end
  end
end

timers.check_timers = function(ticks)
  for mod_id, mod_timers in pairs(timers.registry) do
    for tag, timer in pairs(mod_timers) do
      -- check trigger time
      if ticks >= (timer.created_at + timer.ticks) then
        -- exec callback
        exec_timer_callback(timer.mod_id, timer.callback)

        -- periodic/one_time logic
        if timer.timer_type == "one_time" then
          -- remove timer from db, registry
          mod_timers[tag] = nil

          local state = require('ck.fallout2.state')
          if state.db.timers and state.db.timers[mod_id] then
            state.db.timers[mod_id][tag] = nil

            if next(state.db.timers[mod_id]) == nil then
              state.db.timers[mod_id] = nil
            end
          end
        elseif timer.timer_type == "periodic" then
          timer.created_at = game_time.get_time()
        end
      end
    end
  end
end

return timers
