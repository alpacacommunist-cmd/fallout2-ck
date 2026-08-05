local ffi = require("ffi")

local proto = {}

proto.types = {
  ITEM = 0,
  CRITTER = 1,
}

function proto.create_item(source_pid, lua_tag)
  local pid = ffi.C.ck_proto_register(source_pid, proto.types.ITEM, lua_tag)
  if pid == -1 then
    error("Failed to create custom item prototype for " .. tostring(lua_tag))
  end

  return {
    pid = pid,
    set_cost = function(self, cost)
      ffi.C.ck_proto_set_item_cost(self.pid, cost)
      return self
    end,
    set_weight = function(self, weight)
      ffi.C.ck_proto_set_item_weight(self.pid, weight)
      return self
    end
  }
end

return proto
