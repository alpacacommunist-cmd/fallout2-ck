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

function gameTime.isNight()
  local hour = gameTime.getHour()

  return hour >= 20 or hour < 6
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

  return 'night'
end

return gameTime
