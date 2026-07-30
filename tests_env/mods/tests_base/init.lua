local ffi     = require 'ffi'
local monitor = require('ck.fallout2.monitor')
local map     = require('ck.fallout2.map')

local function assert_ok(condition, message)
  if not condition then
    log.error("ASSERT FAILED: " .. tostring(message))
    os.exit(1)
  end
end

local _timer = { active = false, target_time = 0, callback = nil }
local function wait_and_run(seconds, callback)
  if type(callback) ~= "function" then return end

  _timer.callback = callback
  _timer.target_time = os.clock() + (seconds or 1.0)
  _timer.active = true

  log.info(string.format("Automation: Scheduled task in %.1f seconds...", seconds))
end

-- runs every 2 seconds though
events.on('onMapUpdate', function()
  if _timer.active and os.clock() >= _timer.target_time then
    _timer.active = false

    local callback = _timer.callback
    _timer.callback = nil

    callback()
  end
end)

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

    wait_and_run(2.0, function()
      log.info("Automation: Delay finished, switching to 'combat' suite...")
      ffi.C.ck_testing_set_current_suite('combat')
      ffi.C.ck_scripting_load_game_slot(1)
    end)
  end

  if current_suite == "combat" then
    monitor.print("Running COMBAT integration test...")
    wait_and_run(1.0, function()
      log.info("Automation: All tests completed. Exiting.")
      os.exit(0)
    end)
  end
end)

