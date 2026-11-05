local ffi      = require('ffi')
local ck       = require('ck')
local encoding = require('ck.fallout2.encoding')

local objects  = require('ck.fallout2.objects')
local items    = require('ck.fallout2.objects.items')

local log      = ck.log.new('system/ffi/meta/object.lua')

local ObjectClass, CritterClass
local FalloutObjectMethods = {}

function FalloutObjectMethods:is_critter()
  return bit.rshift(self.pid, 24) == 1
end

function FalloutObjectMethods:give_item(item_pid, count)
  count = count or 1
  items.add(self, item_pid, count) 
end

function FalloutObjectMethods:get_name()
  local c_str = ffi.C.ck_object_get_name(self)
  return c_str ~= nil and encoding.cp1251_to_utf8(c_str) or "Unknown"
end

function FalloutObjectMethods:restore()
  return ffi.C.ck_registry_restore_modified_object(self)
end

function FalloutObjectMethods:bind()
  local current_mod_id = ck.tools.current_mod_id()

  if objects.registry[current_mod_id] and objects.registry[current_mod_id][self.id] then
    -- current mod owns registry entry
    return objects.registry[current_mod_id][self.id]
  end

  if not ffi.C.ck_registry_modify_object(self) then
    return nil 
  end

  if not ObjectClass then
    ObjectClass  = require('ck.fallout2.classes.object')
    CritterClass = require('ck.fallout2.classes.critter')
  end

  local bound_instance
  if self:is_critter() then
    bound_instance = CritterClass.new(self)
  else
    bound_instance = ObjectClass.new(self)
  end

  return bound_instance
end

ffi.metatype("struct fallout_Object", {
    __index = FalloutObjectMethods
})
