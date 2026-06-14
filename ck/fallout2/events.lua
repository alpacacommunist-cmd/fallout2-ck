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
function events.on(event_name, callback)
  if events.listeners[event_name] then
    table.insert(events.listeners[event_name], callback)
  else
    print("[CK Warning] Attempted to subscribe to unknown event: " .. tostring(event_name))
  end
end

function events.emit(event_name, ...)
  local listeners = events.listeners[event_name]

  if not listeners then
    print("[CK Warning] Attempted to emit unknown event: " .. tostring(event_name))
    return
  end

  local args = { ... }

  for index, callback in ipairs(listeners) do
    local ok, err = xpcall(function()
      callback(unpack(args))
    end, debug.traceback)

    if not ok then
      print(string.format("[CK Events] ERROR in event '%s' callback #%d:\n%s", event_name, index, err))
    end
  end
end

function events.clear()
  for event_name in pairs(events.listeners) do
    events.listeners[event_name] = {}
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

function ckOnDialogStart(id)
  print("[CK Events] Dialogue started: " .. tostring(id))

  ck.dialog.set_reply("Hello, traveler.")
  ck.dialog.add_option("Who are you?", 49)
  ck.dialog.add_option("Goodbye.")

  local choice = ck.dialog.go()
  print("[CK Dialog] choice = " .. tostring(choice))

  ck.dialog.exit()
end

-- 
-- C <-> lua entry points END
--

return events
