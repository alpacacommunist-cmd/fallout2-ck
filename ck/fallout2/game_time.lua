-- ck/fallout2/game_time.lua
local ffi = require("ffi")

local utils = require('ck.system.utils')

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

-- Timer types
game_time.timer_types = {
  ["one_time"] = "one_time",
  ["repeat"]   = "repeat"
}

-- Timers registry
-- { "temple_of_trials" = { "timer_1" = {}, "timer_2" = {} ... } }
game_time.timers = {}

game_time.generate_timer_id = function(mod_id)
  local count = 0
  if game_time.timers[mod_id] then
    for _ in pairs(game_time.timers[mod_id]) do count = count + 1 end
  end

  return mod_id .. "_timer_" .. count
end

game_time.register_timer = function(tag, timer_type, ticks, callback, params)
  local mod_id = ffi.C.ck_get_current_mod_id()
  local current_time = ffi.C.ck_game_time_get_time()

  ticks = ticks or current_time
  timer_type = game_time.timer_types[type] or game_time.timer_types.one_time

  if not tag or utils.is_blank(tag) then
    tag = game_time.generate_timer_id(mod_id)
  end

  -- check existing timer in registry
  if game_time.timers[mod_id] and game_time.timers[mod_id][tag] then
    local timer = game_time.timers[mod_id][tag]
  else
    if type == "one_time" and current_time > ticks then
      -- exec callback
    else
      -- new timer, write to registry
      -- and save to state
      local state = require('ck.fallout2.state')

      game_time.timers[mod_id] = game_time.timers[mod_id] or {}
      state.db.timers[mod_id] = state.db.timers[mod_id] or {}

      local timer = { tag = tag, type = type, ticks = ticks, callback = callback, params = params }

      game_time.timers[mod_id][tag] = timer
      state.db.timers[mod_id][tag] = { created_at = current_time }
    end
  end
end

function game_time.get_total_days()
  return math.floor(ffi.C.ck_game_time_get_time() / (10 * 60 * 60 * 24))
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
