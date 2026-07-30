local ffi     = require 'ffi'
local monitor = require('ck.fallout2.monitor')
local map     = require('ck.fallout2.map')

ffi.cdef[[
const char* ck_testing_get_current_suite();

int ck_scripting_load_game_slot(int slot);
]]

local function assert_ok(condition, message)
  if not condition then
    log.error("ASSERT FAILED: " .. tostring(message))
    os.exit(1)
  end
end

events.on('onMapEnter', function()
  local current_suite = ffi.string(ffi.C.ck_testing_get_current_suite())

  log.info("Entering map under test suite: " .. current_suite)

  if current_suite == "base" then
    monitor.print("Running BASE integration test...")

    local found = map.find_by_pid(16777219)
    assert_ok(#found > 0, "Klint NPC not found on the map!")

    local klint_raw = found[1]
    assert_ok(klint_raw:is_critter() == true, "Klint OOP object thinks it's not a critter!")

    local klint = klint_raw:bind()
    assert_ok(klint ~= nil, "Failed to bind Klint to Lua OOP class!")

    klint:float_message("Hello from automated test!", 1)

    log.info("TEST 'base' passed!")

    os.exit(0)

    -- ffi.C.ck_engine_load_game_slot(1)
  end

  if current_suite == "combat" then
    monitor.print("Running COMBAT integration test...")
    os.exit(0)
  end
end)
