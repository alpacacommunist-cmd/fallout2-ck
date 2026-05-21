-- ck/fallout2/events.lua

local events = {
    -- listeners stack
    listeners = {
        onGameStart = {},
        onDayPassed = {}
    }
}

-- Public mod API
-- Allows mod events subscription (fallout2.events.on('onGameStart'))
function events.on(eventName, callback)
    if events.listeners[eventName] then
        table.insert(events.listeners[eventName], callback)
    else
        print("[CK Warning] Attempted to subscribe to unknown event: " .. tostring(eventName))
    end
end

function events.emit(eventName)
    local listeners = events.listeners[eventName]

    if not listeners then
        print("[CK Warning] Attempted to emit unknown event: " .. tostring(eventName))
        return
    end

    for _, callback in ipairs(listeners) do
        pcall(callback)
    end
end


-- 
-- Single C entry point
--

-- ckHookOnGameStart (C) calls it when game started (interface initiated)
function ckOnGameStart()
    print("[CK Events] Engine signaled: Game Start! Firing listeners...")
    events.emit('onGameStart')
end

-- 
-- Single C entry point END
--


return events
