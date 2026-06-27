local ffi = require("ffi")
ffi.cdef[[
  typedef struct { int lua_id; const char* mod_id; } CritterLua;

  CritterLua ck_critter_register(int pid, int tile, const char* tag);
]]

local log = ck.log.new('objects/critters.lua')

local CritterClass = require('ck.fallout2.classes.critter')
local map          = require('ck.fallout2.map')
local state        = require('ck.fallout2.state')

local critters = {}

function critters.register(tag, pid, tile, config)
  local critter_data = ffi.C.ck_critter_register(pid, tile, tag);

  if critter_data.lua_id == -1 then
    print("Failed to register critter (FFI)!")
    return nil
  end

  return CritterClass.new(critter_data.lua_id, config, tag, ffi.string(critter_data.mod_id));
end

function critters.create(pid, tile, config)
  local critter_data = ffi.C.ck_critter_register(pid, tile, nil)

  if critter_data.lua_id == -1 then
    log.error("Failed to create critter (FFI)!")
    return nil
  end

  return CritterClass.new(critter_data.lua_id, config, nil, ffi.string(critter_data.mod_id));
end

return critters
