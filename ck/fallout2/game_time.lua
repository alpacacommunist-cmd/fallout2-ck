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
        day = gameTime.getDay(), month = gameTime.getMonth(),
        year = gameTime.getYear(), hour = gameTime.getHour()
    }
end

return gameTime
