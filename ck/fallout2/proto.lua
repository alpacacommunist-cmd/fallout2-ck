local ffi = require("ffi")
local log = ck.log.new('proto.lua')

local proto = {}

proto.PROC_NAMES = {
  [3]  = "description",
  [4]  = "pickup",
  [6]  = "use",
  [11] = "talk",
  -- [12] = "critter", -- doesn't work bc gScriptsLists, using own on_map_update from ck_script.cc
  [13] = "combat",
  [14] = "damage",
  [18] = "destroy",
  [21] = "look_at",
  [22] = "timed",
  -- [23] = "map_update", -- doesn't work bc gScriptsLists, using own on_map_update from ck_script.cc
  [24] = "push"
}

proto.types = {
  ITEM = 0,
  CRITTER = 1,
}

proto.registry = {}

function proto.register_prototype(source_pid, lua_tag, config)
  local ffi_data = ffi.new("CustomProtoFFI")

  ffi_data.price  = config.price  or 0
  ffi_data.weight = config.weight or 0

  ffi_data.name   = config.name or ""
  ffi_data.description   = config.description or ""

  ffi_data.inv_fid = config.inv_fid or -1

  local pid = ffi.C.ck_proto_register(source_pid, proto.types.ITEM, lua_tag, ffi_data)
  if pid == -1 then
    error("Failed to declare custom prototype: " .. tostring(lua_tag))
  end

  local proto_instance = { tag = lua_tag, pid = pid, sid = -1, handlers = {} }
  function proto_instance:bind()
    ffi.C.ck_proto_bind(self.pid)
    return self
  end

  function proto_instance:on(event_name, callback)
    self.handlers[event_name] = callback

    return proto_instance
  end

  function proto_instance:_handle_proc(proc_id, fixed_param)
    local event_name = proto.PROC_NAMES[proc_id]
    if not event_name then return false end

    if self.handlers[event_name] then
      local result = self.handlers[event_name](self, fixed_param)

      if result ~= nil then return result end
    end
  end

  proto.registry[pid] = proto_instance

  return proto_instance
end

return proto
