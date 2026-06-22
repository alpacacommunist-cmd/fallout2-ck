local ffi = require("ffi")

ffi.cdef[[
  void ck_critter_float_msg(int lua_id, const char* text, int msg_type);

  int ck_anim_begin(void* ptr, int weapon_ready);
  int ck_anim_move_to(void* ptr, int tile, int elevation);
  int ck_anim_play(void* ptr, int anim_id);
  bool ck_critter_is_busy(void* ptr);
  int ck_anim_end();
]]

local dialogue = require('ck.fallout2.dialogue')
local log      = require('ck.fallout2.log')

local Object = require("ck.fallout2.classes.object")

local Critter = {}
setmetatable(Critter, { __index = Object })
Critter.__index = Critter

function Critter.new(lua_id, config)
  local self = Object.new(lua_id, config)
  setmetatable(self, Critter)

  self.active_behavior = nil

  return self
end

function Critter:float_message(text, type)
  ffi.C.ck_critter_float_msg(self.id, text, type)
end

function Critter:_handle_proc(proc_id)
  local event_name = Object.PROC_NAMES[proc_id]
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

function Critter:is_busy()
  return ffi.C.ck_critter_is_busy(self.c_ptr)
end

function Critter:animate(request_options)
  request_options = request_options or 0

  if ffi.C.ck_anim_begin(self.c_ptr, request_options) ~= 0 then
    print("[CK Error] Failed to begin animation sequence")
    return nil
  end

  local builder = {}
  local obj_ptr = self.c_ptr

  function builder:walk_to(target_tile, elevation)
    ffi.C.ck_anim_move_to(obj_ptr, target_tile, elevation or 0)
    return self
  end

  function builder:play(anim_id)
    ffi.C.ck_anim_play(obj_ptr, anim_id)
    return self
  end

  function builder:submit() ffi.C.ck_anim_end() end

  return builder
end

function Critter:_handle_map_update(current_ticks)
  if self.handlers['map_update'] then
    self.handlers['map_update'](self)
  end

  if self.active_behavior then
    self.active_behavior(self, current_ticks)
  end
end

return Critter
