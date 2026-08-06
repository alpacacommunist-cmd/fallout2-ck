local ffi = require("ffi")

local proto = {}

proto.types = {
  ITEM = 0,
  CRITTER = 1,
}

function proto.register_prototype(source_pid, lua_tag, config)
  local ffi_data = ffi.new("CustomProtoFFI")

  ffi_data.price  = config.price  or 0
  ffi_data.weight = config.weight or 0

  local initial_pid = ffi.C.ck_proto_register(source_pid, proto.types.ITEM, lua_tag, ffi_data)
  if initial_pid == -1 then
    error("Failed to declare custom prototype: " .. tostring(lua_tag))
  end

  return setmetatable({ tag = lua_tag }, {
    __index = function(t, key)
      if key == "pid" then
        return ffi.C.ck_proto_get_pid_by_tag(t.tag)
      end
    end
  })
end

return proto
