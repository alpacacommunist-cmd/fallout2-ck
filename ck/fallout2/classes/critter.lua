local ffi = require("ffi")

ffi.cdef[[
  void ck_critter_float_msg(int lua_id, const char* text, int msg_type);
]]

local dialogue = require('ck.fallout2.dialogue')
local log      = require('ck.fallout2.log')
local objects  = require('ck.fallout2.objects')

local Critter = {}
Critter.__index = Critter

function Critter.new(lua_id, config)
  local self = setmetatable({}, Critter)

  self.id = lua_id
  self.name = config.name
  self.description = config.description
  self.handlers = {}

  return self
end


function Critter:on(event_name, callback)
  self.handlers[event_name] = callback

  return self
end

function Critter:float_message(text, type)
  ffi.C.ck_critter_float_msg(self.id, text, type)
end

function Critter:_handle_proc(proc_id)
  local event_name = objects.PROC_NAMES[proc_id]
  if not event_name then return false end

  if self.handlers[event_name] then
    if self.handlers[event_name](self) ~= false then
      print(event_name)
      return true
    end
  end

  if event_name == "look_at" then
    if log and log.print then log.print(self.name) end

    return true
  elseif event_name == "description" then
    if log and log.print then log.print(self.description) end

    return true
  elseif event_name == "talk" then
    if dialogue and dialogue.start then
      dialogue.start(self.id)

      return true
    end
  end

  return false
end

-- function Critter:walk_to(target_tile)
--   fallout.reg_anim_begin(0)
--   fallout.animationRegisterMoveToTile(self.id, target_tile, 0, -1, 0)
--   fallout.reg_anim_end()
-- end

return Critter
