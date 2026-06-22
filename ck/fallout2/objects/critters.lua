local ffi     = require("ffi")

local CritterClass = require("ck.fallout2.classes.critter")

ffi.cdef[[
  int ck_critter_register(int pid, int tile);
]]

local critters = {}

function critters.register(pid, tile, config)
  local name = config.name
  local desc = config.description

  local lua_id = ffi.C.ck_critter_register(pid, tile)
  if lua_id == -1 then
    print("[CK Error] Failed to register C++ critter via FFI!")
    return nil
  end

  local critter_instance = CritterClass.new(lua_id, config)

  return critter_instance
end

return critters
