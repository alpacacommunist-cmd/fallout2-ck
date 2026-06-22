local ffi = require("ffi")

ffi.cdef[[
  int ck_object_get_tile(int lua_id);
  int ck_object_get_sid(int lua_id);
]]

local objects = require('ck.fallout2.objects')

local Object = {}
Object.__index = Object

Object.PROC_NAMES = objects.PROC_NAMES

function Object.new(lua_id, config)
  local self = setmetatable({}, Object)

  self.id          = lua_id
  self.sid         = ffi.C.ck_object_get_sid(lua_id)

  self.name        = config.name
  self.description = config.description

  self.handlers = {}

  objects.registry[lua_id] = self

  return self
end

function Object:on(event_name, callback)
  self.handlers[event_name] = callback

  return self
end

function Object:tile()
  return ffi.C.ck_object_get_tile(self.id)
end

return Object
