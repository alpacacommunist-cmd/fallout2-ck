local ffi = require('ffi')
local ck = require('ck')

local tests = {}

function tests.current_suite()
  return ffi.string(ffi.C.ck_testing_get_current_suite())
end

function tests.set_suite(name)
  ffi.C.ck_testing_set_current_suite(name)
end

function tests.load_game_slot(slot)
  ffi.C.ck_load_game_slot(slot)
end

function tests.clock()
  return os.clock()
end

function tests.exit(status)
  os.exit(status)
end

return tests
