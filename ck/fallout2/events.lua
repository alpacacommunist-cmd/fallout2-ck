-- ck/fallout2/events.lua

local events = {
  -- listeners stack
  listeners = {
    onGameStart = {},
    onDayPassed = {},
    onHourPassed = {},
    onGameLoaded = {},
    onTimeAdvance = {},
    onMapEnter = {}
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

function events.emit(eventName, ...)
  local listeners = events.listeners[eventName]

  if not listeners then
    print("[CK Warning] Attempted to emit unknown event: " .. tostring(eventName))
    return
  end

  for index, callback in ipairs(listeners) do
    local args = { ... }

    local success, err = xpcall(
        function() callback(table.unpack(args)) end,
        debug.traceback
    )

    if not success then
        print(
            "[CK Events] ERROR in event '" .. tostring(eventName) ..
            "' callback #" .. tostring(index) .. ":\n" .. tostring(err)
        )
    end
  end

end

--
-- C -> Lua entry points
--

-- ckHookOnGameStart (C) calls it when game started (interface initiated)
function ckOnGameStart()
  print("[CK Events] Engine signaled: Game Start! Firing listeners...")
  events.emit('onGameStart')
end

function ckOnGameLoaded()
  print("[CK Events] Engine signaled: Game Loaded!")
  events.emit('onGameLoaded')
end

function ckOnDayPassed()
  print("[CK Events] Engine signaled: Day Passed!")
  events.emit('onDayPassed')
end

function ckOnHourPassed()
  print("[CK Events] Engine signaled: Hour Passed!")
  events.emit('onHourPassed')
end

function ckOnMapEnter()
  print("[CK Events] Engine signaled: Map Enter!")
  events.emit('onMapEnter')
end

function ckOnTimeAdvance(hours, minutes)
  print("[CK Events] Time Advanced on " .. tostring(hours) .. " h. and " .. tostring(minutes) .. " minutes")
  events.emit('onTimeAdvance', hours, minutes)
end

-- 
-- C <-> lua entry points END
--

return events
