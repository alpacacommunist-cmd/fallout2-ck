-- ck/fallout2/log.lua

local log = {}

-- wraps C functions
function log.print(message)
  -- see if the whole "require" facade even works
  print("[Lua Facade Debug]: " .. tostring(message))

  -- calls C-registered functions
  ckLogPrint(message)
end

-- returns log.object
return log
