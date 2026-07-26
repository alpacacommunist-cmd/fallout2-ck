local ffi = require("ffi")

local log = ck.log.new('classes/object')

local objects = require('ck.fallout2.objects')
local items   = require('ck.fallout2.objects.items')
local monitor = require('ck.fallout2.monitor')

local Object = {}
Object.__index = Object

Object.PROC_NAMES = objects.PROC_NAMES

function Object.new(id, config, mod_id)
  local self = setmetatable({}, Object)

  self.id          = id
  self.mod_id      = mod_id
  self.sid         = ffi.C.ck_object_get_sid(id)
  self.c_ptr       = ffi.C.ck_object_get_ptr(id)

  self.name        = config.name
  self.description = config.description

  self.handlers = {}

  objects.registry[id] = self

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
      monitor.print(self.name)

      return true
    end

  elseif event_name == "description" then
    if self.description then
      monitor.print(self.description)

      return true
    end

  elseif event_name == "damage" then
    log.info('damaged object: ' .. tostring(self.id))

    return false

  elseif event_name == "destroy" then
    log.info('Object destroyed: ' .. tostring(self.id))
    -- ffi.C.ck_critter_kill(self.id)
    --
    return false
  end

  return false
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
