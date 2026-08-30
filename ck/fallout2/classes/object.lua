local ffi = require("ffi")

local log = ck.log.new('classes/object')
local utils = require('ck.system.utils')

local objects = require('ck.fallout2.objects')
local items   = require('ck.fallout2.objects.items')
local monitor = require('ck.fallout2.monitor')
local state = require('ck.fallout2.state')

local Object = {}
Object.__index = Object

Object.PROC_NAMES = objects.PROC_NAMES

function Object.new(lua_id, tag, mod_id, config)
  local self = setmetatable({}, Object)

  self.lua_id      = lua_id
  self.mod_id      = mod_id
  self.tag         = tag

  self.modified    = config.modified or false

  self.c_ptr       = ffi.C.ck_object_get_ptr(self.lua_id)
  self.sid         = ffi.C.ck_object_get_sid(self.c_ptr)

  self.name        = config.name
  self.description = config.description

  self.elevation = config.elevation

  self.handlers = {}

  local object_state = self:state()
  if object_state and object_state.inventory then
    for pid, qty in pairs(object_state.inventory) do
      self:give_item(pid, qty)
    end
  end

  objects.registry[lua_id] = self

  return self
end

function Object:on(event_name, callback)
  self.handlers[event_name] = callback

  return self
end

function Object:emit(event_name, ...)
  if self.handlers[event_name] then
    local ok, err = xpcall(self.handlers[event_name], debug.traceback, self, ...)

    if not ok then
      log.error(string.format("in object '%s' on event '%s':\n%s", tostring(self.tag), event_name, err))
    end
  end

  return self
end

function Object:_handle_proc(proc_id, fixed_param)
  local event_name = Object.PROC_NAMES[proc_id]
  if not event_name then return false end

  -- check custom callbacks eg alice:on, door:on etc
  if self.handlers[event_name] then
    local result = self.handlers[event_name](self, fixed_param)

    if result ~= nil then return result end
  end

  -- defaults
  if event_name == "look_at" then
    if self.name then
      if self:type() == 'critter' and self.is_dead then
        monitor.print(ck_t('you_see_dead', self.name))
      else
        monitor.print(ck_t('you_see', self.name))
      end

      return true
    end

  elseif event_name == "description" then
    if self.description and not self.is_dead then
      monitor.print(self.description)

      return true
    end

  elseif event_name == "damage" then
    log.info('damaged object: ' .. tostring(self.lua_id))

    return false

  elseif event_name == "destroy" then
    if (ffi.C.ck_mods_reload_in_progress()) then return false end

    log.info('Object destroyed: ' .. tostring(self.lua_id))

    if self:type() == 'critter' then
      log.info('Critter destroyed: ' .. tostring(self.lua_id))

      self.is_dead = true
      ffi.C.ck_critter_kill(self.lua_id)
      -- if (self.tag) then -- only for tagged critters
      --   objects.registry[self.lua_id] = nil
      --
      --   self.lua_id = corpse_lua_id
      --   self.c_ptr  = ffi.C.ck_object_get_ptr(self.lua_id)
      --   self.sid    = ffi.C.ck_object_get_sid(self.c_ptr)
      --
      --   objects.registry[corpse_lua_id] = self
      -- end

      return true
    end

    return false
  end

  return false
end

function Object:restore()
  return ffi.C.ck_registry_restore_modified_object(self.c_ptr)
end

function Object:float_message(text, type)
  return ffi.C.ck_object_float_msg(self.c_ptr, text, type)
end

function Object:id()
  return ffi.C.ck_object_get_id(self.c_ptr)
end

function Object:tile()
  return ffi.C.ck_object_get_tile(self.c_ptr)
end

function Object:get_name()
  return ffi.string(ffi.C.ck_object_get_name(self.c_ptr))
end

function Object:type()
  return objects.TYPES[ffi.C.ck_object_get_type(self.c_ptr)]
end

function Object:give_item(item_pid, count, persistent)
  count      = count or 1
  persistent = persistent or false

  return items.add(self.c_ptr, item_pid, count, persistent)
end

function Object:item_count(item_pid)
  return items.count(self.c_ptr, item_pid)
end

function Object:has_item(item_pid)
  return self:item_count(item_pid) > 0
end

function Object:state()
  if (not self.tag or not self.mod_id) then return nil end

  local map_id = ffi.C.ck_map_get_id()
  if map_id == -1 then return nil end

  local maps = state.db.maps
  maps[map_id] = maps[map_id] or {}
  maps[map_id][self.mod_id] = maps[map_id][self.mod_id] or {}
  maps[map_id][self.mod_id][self.tag] = maps[map_id][self.mod_id][self.tag] or {}

  return maps[map_id][self.mod_id][self.tag]
end

function Object:state_readonly()
  local original_state = self:state()
  if not original_state then return nil end

  return utils.make_readonly(original_state, self.tag)
end

function Object:inventory_table()
  local inventory_table = {}
  local items_count = ffi.C.ck_total_inventory_count(self.c_ptr)

  local pid_ptr = ffi.new("int[1]")
  local qty_ptr = ffi.new("int[1]")

  for index = 0, items_count - 1 do
    if ffi.C.ck_get_inventory_item(self.c_ptr, index, pid_ptr, qty_ptr) then
      local pid = pid_ptr[0]
      local qty = qty_ptr[0]

      inventory_table[pid] = (inventory_table[pid] or 0) + qty
    end
  end

  return inventory_table
end

function Object:has_inventory()
  local current_state = self:state()
  if not current_state then return false end

  local inventory = current_state.inventory

  return type(inventory) == "table" and next(inventory) ~= nil
end

return Object
