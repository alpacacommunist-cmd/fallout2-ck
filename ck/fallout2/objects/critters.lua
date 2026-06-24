local ffi     = require("ffi")

local CritterClass = require("ck.fallout2.classes.critter")

ffi.cdef[[
  int ck_critter_register(int pid, int tile, const char* tag);
]]

local critters = {}

function critters.register(tag, pid, tile, config)
  local lua_id = ffi.C.ck_critter_register(pid, tile, tag)
  if lua_id == -1 then
    print("[CK Error] Failed to register critter (FFI)!")
    return nil
  end

  local critter_instance = CritterClass.new(lua_id, config, tag)

  return critter_instance
end

function critters.create(pid, tile, config)
  local lua_id = ffi.C.ck_critter_register(pid, tile, nil)
  if lua_id == -1 then
    print("[CK Error] Failed to create critter (FFI)!")
    return nil
  end

  local critter_instance = CritterClass.new(lua_id, config)

  return critter_instance
end

return critters
