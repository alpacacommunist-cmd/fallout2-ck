local ffi = require("ffi")

ffi.cdef[[
  int ck_object_get_tile(int lua_id);
  int ck_object_get_sid(int lua_id);
  void* ck_object_get_ptr(int lua_id);
]]

local objects = require('ck.fallout2.objects')
local items   = require('ck.fallout2.objects.items')

local Object = {}
Object.__index = Object

Object.PROC_NAMES = objects.PROC_NAMES

function Object.new(lua_id, config, mod_id)
  local self = setmetatable({}, Object)

  self.id          = lua_id
  self.mod_id      = mod_id or "unknown"
  self.sid         = ffi.C.ck_object_get_sid(lua_id)
  self.c_ptr       = ffi.C.ck_object_get_ptr(lua_id)

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

function Object:give_item(item_pid, count)
  count = count or 1
  return items.add(self.c_ptr, item_pid, count)
end

function Object:item_count(item_pid)
  return items.count(self.c_ptr, item_pid)
end

function Object:has_item(item_pid)
  return self:item_count(item_pid) > 0
end

return Object
