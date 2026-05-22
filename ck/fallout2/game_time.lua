-- ck/fallout2/game_time.lua

local gameTime = {}

function gameTime.getYear()
  return ckGetYear()
end

function gameTime.getDay()
  return ckGetDay()
end

function gameTime.getMonth()
  return ckGetMonth()
end

function gameTime.getHour()
  return ckGetHour()
end

function gameTime.getDate()
  return {
    day = gameTime.getDay(),
    month = gameTime.getMonth(),
    year = gameTime.getYear(),
    hour = gameTime.getHour()
  }
end

function gameTime.getTotalDays()
  return ckGetTotalDays()
end

function gameTime.getTimeOfDay()
  local hour = gameTime.getHour()

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

function gameTime.isMorning()
  return gameTime.getTimeOfDay() == 'morning'
end

function gameTime.isDay()
  return gameTime.getTimeOfDay() == 'day'
end

function gameTime.isEvening()
  return gameTime.getTimeOfDay() == 'evening'
end

function gameTime.isNight()
  return gameTime.getTimeOfDay() == 'night'
end

function gameTime.hasDaysPassed(days, sinceDay)
  return gameTime.getTotalDays() - sinceDay >= days
end

function gameTime.getSeason()
  local month = gameTime.getMonth()

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

function gameTime.isSeason(season)
  return gameTime.getSeason() == season
end

function gameTime.getDayOfWeek()
  local totalDays = gameTime.getTotalDays()

  local days = {
    'monday',
    'tuesday',
    'wednesday',
    'thursday',
    'friday',
    'saturday',
    'sunday'
  }

  return days[(totalDays % 7) + 1]
end

return gameTime
