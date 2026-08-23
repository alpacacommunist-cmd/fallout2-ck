local ffi = require("ffi")

local log = ck.log.new('objects/critters.lua')

local CritterClass = require('ck.fallout2.classes.critter')
local map          = require('ck.fallout2.map')
local state        = require('ck.fallout2.state')

local critters = {}

function critters.register(tag, pid, tile, config)
  config = config or {}
  config.elevation = config.elevation or ffi.C.ck_current_elevation()

  local params = ffi.new("CritterLuaProtoParams")
  params.name        = config.name or ""
  params.description = config.description or ""

  local critter_data = ffi.C.ck_critter_spawn(pid, tile, config.elevation, tag, params);

  if critter_data.lua_id == -1 then
    print("Failed to register critter (FFI)!")
    return nil
  end

  return CritterClass.new(critter_data.lua_id, config, tag, ffi.string(critter_data.mod_id));
end

function critters.create(pid, tile, config)
  config = config or {}
  config.elevation = config.elevation or ffi.C.ck_current_elevation()

  local critter_data = ffi.C.ck_critter_spawn(pid, tile, config.elevation, nil, nil)

  if critter_data.lua_id == -1 then
    log.error("Failed to create critter (FFI)!")
    return nil
  end

  return CritterClass.new(critter_data.lua_id, config, nil, ffi.string(critter_data.mod_id));
end

return critters
