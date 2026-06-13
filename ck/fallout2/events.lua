-- ck/fallout2/events.lua
local unpack = table.unpack or unpack

local events = {
  -- listeners stack
  listeners = {
    onModReload = {},
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

  local args = { ... }

  for index, callback in ipairs(listeners) do
    local ok, err = xpcall(function()
      callback(unpack(args))
    end, debug.traceback)

    if not ok then
      print(string.format("[CK Events] ERROR in event '%s' callback #%d:\n%s", eventName, index, err))
    end
  end
end

function events.clear()
  for eventName in pairs(events.listeners) do
    events.listeners[eventName] = {}
  end

  print("[CK Events] Cleared all listeners.")
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
