-- ck/fallout2/game_time.lua
local ffi = require("ffi")

local utils = require('ck.system.utils')
local log   = ck.log.new('game_time.lua')

local game_time = {}

game_time.get_year  = ffi.C.ck_game_time_get_year
game_time.get_day   = ffi.C.ck_game_time_get_day
game_time.get_month = ffi.C.ck_game_time_get_month
game_time.get_hour  = ffi.C.ck_game_time_get_hour

function game_time.get_date()
  return {
    day   = game_time.get_day(),
    month = game_time.get_month(),
    year  = game_time.get_year(),
    hour  = game_time.get_hour()
  }
end

function game_time.get_time()
  return ffi.C.ck_game_time_get_time()
end

-- time to ticks helper
-- 10 ticks per second
game_time.in_ticks = {
  days = function(number_of_days)
    return number_of_days * (60 * 60 * 24) * 10
  end,

  hours = function(number_of_hours)
    return number_of_hours * (60 * 60) * 10
  end,

  minutes = function(number_of_minutes)
    return number_of_days * 60 * 10
  end,

  seconds = function(number_of_seconds)
    return number_of_seconds * 10
  end
}

game_time.from_now = function(ticks)
  return game_time.get_time() + ticks
end

-- Timer types
game_time.timer_types = {
  ["one_time"] = "one_time",
  ["repeat"]   = "repeat"
}

-- Timers registry
-- { "temple_of_trials" = { "timer_1" = {}, "timer_2" = {} ... } }
game_time.timers = {}

-- mod_id + "_timer_" + count
game_time.generate_timer_id = function(mod_id)
  local count = 0
  if game_time.timers[mod_id] then
    for _ in pairs(game_time.timers[mod_id]) do count = count + 1 end
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

game_time.register_timer = function(tag, timer_type, ticks, callback, params)
  local mod_id = ffi.string(ffi.C.ck_get_current_mod_id())
  local current_time = game_time.get_time()

  ticks = ticks or 0
  exec_time = current_time + ticks

  timer_type = game_time.timer_types[timer_type] or game_time.timer_types.one_time

  if not tag or utils.is_blank(tag) then
    tag = game_time.generate_timer_id(mod_id)
  end

  log.error("timer_type: %s, ticks: %d, current_time: %d, mod_id: %s", timer_type, ticks, current_time, mod_id)

  -- check existing timer in registry
  if game_time.timers[mod_id] and game_time.timers[mod_id][tag] then
    local timer = game_time.timers[mod_id][tag]
  else
    if timer_type == "one_time" and exec_time <= ticks then
      exec_timer_callback(mod_id, callback)
    else
      -- new timer, write to registry, save to state db
      local state = require('ck.fallout2.state')

      game_time.timers[mod_id] = game_time.timers[mod_id] or {}
      state.db.timers[mod_id]  = state.db.timers[mod_id] or {}

      local timer = {
        tag = tag,
        type = timer_type,
        created_at = current_time,
        ticks = ticks,
        callback = callback,
        mod_id = mod_id,
        params = params
      }

      game_time.timers[mod_id][tag] = timer
      state.db.timers[mod_id][tag]  = { created_at = current_time }
    end
  end
end

game_time.check_timers = function(ticks)
  for mod_id, mod_timers in pairs(game_time.timers) do
    utils.print_table(game_time.timers, log)
    for tag, timer in pairs(mod_timers) do

      -- check trigger time
      if ticks >= (timer.created_at + timer.ticks) then

        -- exec callback
        exec_timer_callback(timer.mod_id, timer.callback)

        -- repeat/one_time logic
        if timer.type == "one_time" then
          -- remove timer from db, registry
          mod_timers[tag] = nil

          local state = require('ck.fallout2.state')
          if state.db.timers and state.db.timers[mod_id] then
            state.db.timers[mod_id][tag] = nil
          end
        elseif timer.type == "repeat" then
          timer.created_at = game_time.get_time()
        end
      end
    end
  end
end

function game_time.get_total_days()
  return math.floor(game_time.get_time() / (10 * 60 * 60 * 24))
end

function game_time.get_time_of_day()
  local hour = game_time.get_hour()

  if hour >= 6 and hour < 12 then return 'morning' end
  if hour >= 12 and hour < 18 then return 'day' end
  if hour >= 18 and hour < 22 then return 'evening' end

  return 'night'
end

function game_time.is_morning() return game_time.get_time_of_day() == 'morning' end
function game_time.is_day()     return game_time.get_time_of_day() == 'day' end
function game_time.is_evening() return game_time.get_time_of_day() == 'evening' end
function game_time.is_night()   return game_time.get_time_of_day() == 'night' end

function game_time.has_days_passed(days, since_day)
  return game_time.get_total_days() - since_day >= days
end

function game_time.get_season()
  local month = game_time.get_month()

  if month >= 3 and month <= 5 then return 'spring' end
  if month >= 6 and month <= 8 then return 'summer' end
  if month >= 9 and month <= 11 then return 'autumn' end

  return 'winter'
end

function game_time.is_season(season)
  return game_time.get_season() == season
end

function game_time.get_day_of_week()
  local total_days = game_time.get_total_days()

  local days = {
    'saturday',
    'sunday',
    'monday',
    'tuesday',
    'wednesday',
    'thursday',
    'friday'
  }

  return days[(total_days % 7) + 1]
end

return game_time
