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

game_time.get_total_days = function()
  return math.floor(game_time.get_time() / (10 * 60 * 60 * 24))
end

game_time.get_time_of_day = function()
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

game_time.has_days_passed = function(days, since_day)
  return game_time.get_total_days() - since_day >= days
end

game_time.get_season = function()
  local month = game_time.get_month()

  if month >= 3 and month <= 5 then return 'spring' end
  if month >= 6 and month <= 8 then return 'summer' end
  if month >= 9 and month <= 11 then return 'autumn' end

  return 'winter'
end

game_time.is_season = function(season)
  return game_time.get_season() == season
end

game_time.get_day_of_week = function()
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
