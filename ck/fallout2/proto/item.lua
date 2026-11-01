local ffi = require('ffi')
local ck = require('ck')
local log = ck.log.new('proto/item.lua')
local proto = require('ck.fallout2.proto')

local ProtoClass = proto.ProtoClass
local ItemProto = setmetatable({}, { __index = ProtoClass })
ItemProto.__index = ItemProto

function ItemProto:new(source_pid, lua_tag, mod_id, config)
  local instance = ProtoClass.new(self, source_pid, lua_tag, mod_id, config)

  instance.price = config.price or -1
  instance.weight = config.weight or -1
  instance.inv_fid = config.inv_fid or -1
  instance.ground_fid = config.ground_fid or -1
  instance.usable = config.usable or false

  instance.handlers = {}

  return instance
end

function ItemProto:to_ffi()
  local ffi_data = ffi.new("ItemProtoFFI")

  ffi_data.lua_tag = self.lua_tag
  ffi_data.name = self.name
  ffi_data.description = self.description

  ffi_data.price  = self.price
  ffi_data.weight = self.weight
  ffi_data.inv_fid    = self.inv_fid
  ffi_data.ground_fid = self.ground_fid
  ffi_data.usable = self.usable

  return ffi_data
end

function ItemProto:register()
  self.pid = ffi.C.ck_item_proto_register(self.source_pid, self:to_ffi())

  if self.pid == -1 then
    log.error("Failed to define prototype: %s", self.lua_tag)
    return nil
  end

  proto.registry[self.pid] = self
  proto.tags_registry[self.mod_id].items[self.lua_tag] = true

  return self
end

function ItemProto:on(event_name, callback)
  self.handlers[event_name] = callback
  return self
end

function ItemProto:_handle_proc(proc_id, fixed_param)
  local event_name = proto.PROC_NAMES[proc_id]
  if not event_name then return false end

  if self.handlers[event_name] then
    local result = self.handlers[event_name](self, fixed_param)
    if result ~= nil then return result end
  end
end

return ItemProto
