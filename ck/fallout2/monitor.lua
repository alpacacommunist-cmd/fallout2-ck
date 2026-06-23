-- ck/fallout2/monitor.lua

local monitor = {}

function monitor.print(message)
  print("[Lua Facade Debug]: " .. tostring(message))

  ckMonitorPrint(message)
end

return monitor
