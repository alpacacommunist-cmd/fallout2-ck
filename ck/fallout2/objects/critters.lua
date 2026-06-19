local ffi     = require("ffi")
local objects = require("ck.fallout2.objects")

local CritterClass = require("ck.fallout2.classes.critter")

ffi.cdef[[
  int ck_map_register_critter(int pid, int tile, const char* name, const char* description);
]]

local critters = {}

function critters.register(pid, tile, config)
  local name = config.name
  local desc = config.description

  local lua_id = ffi.C.ck_map_register_critter(pid, tile, name, desc)
  if lua_id == -1 then
    print("[CK Error] Failed to register C++ critter via FFI!")
    return nil
  end

  local critter_instance = CritterClass.new(lua_id, config)

  objects.registry[lua_id] = critter_instance

  return critter_instance
end

return critters
