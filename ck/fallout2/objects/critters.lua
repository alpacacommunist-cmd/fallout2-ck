local ffi = require("ffi")
ffi.cdef[[
  int ck_critter_register(int pid, int tile, const char* tag, const char* mod_id);
]]

local log = ck.log.new('objects/critters.lua')

local CritterClass = require('ck.fallout2.classes.critter')
local map          = require('ck.fallout2.map')
local state        = require('ck.fallout2.state')

local critters = {}

function critters.register(tag, pid, tile, config)
  local mod_id = events.current_active_mod or "unknown"

  local current_map = map.get_id()
  local stored_data = state.get_stored_object_data(mod_id, current_map, tag)

  if stored_data and stored_data.tile then
    tile = stored_data.tile
  end

  local lua_id = ffi.C.ck_critter_register(pid, tile, tag, mod_id)
  if lua_id == -1 then
    print("[CK Error] Failed to register critter (FFI)!")
    return nil
  end

  local critter_instance = CritterClass.new(lua_id, config, tag, mod_id)
  return critter_instance
end

function critters.create(pid, tile, config)
  local mod_id = events.current_active_mod or "unknown"

  local lua_id = ffi.C.ck_critter_register(pid, tile, nil, mod_id)
  if lua_id == -1 then
    log.error("Failed to create critter (FFI)!")
    return nil
  end

  local critter_instance = CritterClass.new(lua_id, config, nil, mod_id)
  return critter_instance
end

return critters
