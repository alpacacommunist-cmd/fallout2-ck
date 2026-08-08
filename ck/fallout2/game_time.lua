-- ck/fallout2/game_time.lua
local ffi = require("ffi")

ck.game_time = {}

ck.game_time.get_year  = ffi.C.ck_game_get_year
ck.game_time.get_day   = ffi.C.ck_game_get_day
ck.game_time.get_month = ffi.C.ck_game_get_month
ck.game_time.get_hour  = ffi.C.ck_game_get_hour

--
--  FFI END
--

local game_time = {}

game_time.get_year  = ck.game_time.get_year
game_time.get_day   = ck.game_time.get_day
game_time.get_month = ck.game_time.get_month
game_time.get_hour  = ck.game_time.get_hour

function game_time.get_date()
  return {
    day   = game_time.get_day(),
    month = game_time.get_month(),
    year  = game_time.get_year(),
    hour  = game_time.get_hour()
  }
end

function game_time.get_total_days()
  return math.floor(C.ck_game_get_time() / (10 * 60 * 60 * 24))
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
