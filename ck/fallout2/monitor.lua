-- ck/fallout2/monitor.lua

local monitor = {}

local ffi = require('ffi')
local log = require('ck.system.log').new('minitor.log')

function monitor.print(message)
  log.debug(message)

  ffi.C.ck_monitor_print_message(message)
end

return monitor
