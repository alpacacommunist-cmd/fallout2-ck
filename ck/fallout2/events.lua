-- ck/fallout2/events.lua

local events = {
    -- listeners stack
    listeners = {
        onGameStart = {}
    }
}

-- Allows mod events subscription (fallout2.events.on('onGameStart'))
function events.on(eventName, callback)
    if events.listeners[eventName] then
        table.insert(events.listeners[eventName], callback)
    else
        print("[CK Warning] Attempted to subscribe to unknown event: " .. tostring(eventName))
    end
end

-- Single C entry point
-- ckHookOnGameStart (C) calls it on game start
function ckOnGameStart()
    print("[CK Events] Engine signaled: Game Start! Firing listeners...")
    for _, callback in ipairs(events.listeners.onGameStart) do
        pcall(callback)
    end
end

return events
