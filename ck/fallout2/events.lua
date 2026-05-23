-- ck/fallout2/events.lua

local events = {
  -- listeners stack
  listeners = {
    onGameStart = {},
    onDayPassed = {},
    onGameLoaded = {},
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

-- ckHookOnDayPassed (C) calls it when day passes
function ckOnDayPassed()
  print("[CK Events] Engine signaled: Day Passed!")
  events.emit('onDayPassed')
end

function ckOnMapEnter()
  print("[CK Events] Engine signaled: Map Enter!")
  events.emit('onMapEnter')
end

-- 
-- Single C entry point END
--

return events
