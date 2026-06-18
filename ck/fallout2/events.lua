-- ck/fallout2/events.lua
local unpack = table.unpack or unpack

local events = {
  -- listeners stack
  listeners = {
    onModReload = {},
    onGameStart = {},
    onDayPassed = {},
    onHourPassed = {},
    onBeforeGameLoad = {},
    onGameLoaded = {},
    onTimeAdvance = {},
    onDialogStart = {},
    onMapEnter = {}
  },

  current_loading_mod = nil
}

local dialogue = require('ck.fallout2.dialogue')

-- Public mod API
-- Allows mod events subscription (fallout2.events.on('onGameStart'))
function events.on(event_name, callback)
  if not events.listeners[event_name] then
    print("[CK Warning] Attempted to subscribe to unknown event: " .. tostring(event_name))
    return
  end

  table.insert(events.listeners[event_name], {
    mod = events.current_loading_mod or "unknown",
    fn  = callback
  })
end

function events.emit(event_name, ...)
  local entries = events.listeners[event_name]

  if not entries then return end

  local args = { ... }

  for index, entry in ipairs(entries) do
    local ok, err = xpcall(function()
      entry.fn(unpack(args))
    end, debug.traceback)

    if not ok then
      print(string.format("[CK Events] ERROR in mod '%s' on event '%s' (#%d):\n%s", entry.mod, event_name, index, err))
    end
  end
end

function events.clearForMod(mod_name)
  for event_name, entries in pairs(events.listeners) do
    local clean_list = {}

    for _, entry in ipairs(entries) do
      if entry.mod ~= mod_name then
        table.insert(clean_list, entry)
      end
    end

    events.listeners[event_name] = clean_list
  end
  print("[CK Events] Cleared listeners for mod: " .. mod_name)
end

-- ckHookOnGameStart (C) calls it when game started (interface initiated)
function ckOnGameStart()
  print("[CK Events] Engine signaled: Game Start! Firing listeners...")
  events.emit('onGameStart')
end

function ckOnBeforeGameLoad()
  print("[CK Events] Engine signaled: Game Loaded!")
  events.emit('onBeforeGameLoad')
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
  print("[CK Events] Dialog start for npc id: " .. tostring(id))
  events.emit('onDialogStart', id)

  dialogue.start(id)
end

return events
