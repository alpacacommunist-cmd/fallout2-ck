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

return gameTime
