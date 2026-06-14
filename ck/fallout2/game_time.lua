-- ck/fallout2/game_time.lua

local game_time = {}

function game_time.get_year()
  return ck.game_time.get_year()
end

function game_time.get_day()
  return ck.game_time.get_day()
end

function game_time.get_month()
  return ck.game_time.get_month()
end

function game_time.get_hour()
  return ck.game_time.get_hour()
end

function game_time.get_date()
  return {
    day = game_time.get_day(),
    month = game_time.get_month(),
    year = game_time.get_year(),
    hour = game_time.get_hour()
  }
end

function game_time.get_total_days()
  return ck.game_time.get_total_days()
end

function game_time.get_time_of_day()
  local hour = game_time.get_hour()

  if hour >= 6 and hour < 12 then
    return 'morning'
  end

  if hour >= 12 and hour < 18 then
    return 'day'
  end

  if hour >= 18 and hour < 20 then
    return 'evening'
  end

  -- everything after evening is night
  return 'night'
end

function game_time.is_morning()
  return game_time.get_time_of_day() == 'morning'
end

function game_time.is_day()
  return game_time.get_time_of_day() == 'day'
end

function game_time.is_evening()
  return game_time.get_time_of_day() == 'evening'
end

function game_time.is_night()
  return game_time.get_time_of_day() == 'night'
end

function game_time.has_days_passed(days, since_day)
  return game_time.get_total_days() - since_day >= days
end

function game_time.get_season()
  local month = game_time.get_month()

  if month >= 3 and month <= 5 then
    return 'spring'
  end

  if month >= 6 and month <= 8 then
    return 'summer'
  end

  if month >= 9 and month <= 11 then
    return 'autumn'
  end

  return 'winter'
end

function game_time.is_season(season)
  return game_time.get_season() == season
end

function game_time.get_day_of_week()
  local total_days = game_time.get_total_days()

  local days = {
    'monday',
    'tuesday',
    'wednesday',
    'thursday',
    'friday',
    'saturday',
    'sunday'
  }

  return days[(total_days % 7) + 1]
end

return game_time
