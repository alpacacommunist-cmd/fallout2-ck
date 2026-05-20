-- ck/fallout2/events.lua

local events = {
    -- listeners stack
    listeners = {
        onGameStart = {},
        onCheckTimeLimit = {} -- let's try extend game time limit to 50+ years (aka sfall)
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

-- 
-- Single C entry point
--

-- ckHookOnGameStart (C) calls it on game start
function ckOnGameStart()
    print("[CK Events] Engine signaled: Game Start! Firing listeners...")
    for _, callback in ipairs(events.listeners.onGameStart) do
        pcall(callback)
    end
end

-- Hook filter entry point, we use checkTimeLimit as example, work in progress
function ckOnCheckTimeLimit()
    print("[CK Events] Engine asks: Should we extend game time limit?")
    
    -- Runs through listeners. If at least one says true - we return true back
    for _, callback in ipairs(events.listeners.onCheckTimeLimit) do
        local success, result = pcall(callback)
        if success and result == true then
            print("[CK Events] A mod requested to extend the time limit! Sending TRUE to engine.")
            return true -- yes please
        end
    end
    
    return false -- no thanks
end

-- 
-- Single C entry point END
--


return events
