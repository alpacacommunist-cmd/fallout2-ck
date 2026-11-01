local ItemProto = require('ck.fallout2.proto/item')

local ffi = require('ffi')
local ck = require('ck')
local log = ck.log.new('items.lua')

local items = {}

items.count = ffi.C.ck_inventory_count

items.PID_LEATHER_ARMOR = 1
items.PID_METAL_ARMOR   = 2
items.PID_POWER_ARMOR   = 3
items.PID_KNIFE         = 4
items.PID_CLUB          = 5
items.PID_BOTTLE_CAPS   = 41
items.PID_STIMPAK       = 40

function items.add(ptr, pid, count)
  ffi.C.ck_inventory_add(ptr, pid, count)
end

function items.register_prototype(source_pid, lua_tag, config)
  local mod_id = ck.tools.current_mod_id()
  local proto_instance = ItemProto:new(source_pid, lua_tag, mod_id, config)

  if ck.registries.prototype_tags[mod_id].items[proto_instance.lua_tag] then
    log.error("Can't register proto: lua_tag is already taken: %s for mod_id: %s", lua_tag, mod_id)
    return nil
  end

  return proto_instance:register()
end

return items
