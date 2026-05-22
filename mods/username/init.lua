-- mods/username/init.lua
print("[Mod] Loading Username Test Mod...")

local events = require('fallout2.events')
local log    = require('fallout2.log')

events.on('onGameStart', function()
  print("You see: A Giant Ant! Nope! Still LuaJit!") -- linux terminal
  log.print("You see: A Giant Ant! Nope! Still LuaJit!") -- game
end)

function ckOnGameLoaded()
  print("[CK Events] Engine signaled: Game Loaded!") -- linux terminal
  log.print("[CK Events] Save loaded! LuaJIT survived!") -- game

  events.emit('onGameLoaded')
end

events.on('onDayPassed', function()
  print("Another day passed.") -- linux terminal
  log.print("Another day passed.") -- game
end)

