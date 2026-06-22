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

function Critter.new(lua_id, config, tag)
  local self = Object.new(lua_id, config)
  setmetatable(self, Critter)

  self.tag = tag

  self.active_behavior = nil
  self._action_queue   = {}

  self._next_behavior_tick = 0
  self._behavior_interval  = 20

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

function Critter:animate()
  local builder = {}
  local queue = self._action_queue

  function builder:walk_to(target_tile, elevation)
    table.insert(queue, function(critter_ptr)
      ffi.C.ck_anim_begin(critter_ptr, 0)
      ffi.C.ck_anim_move_to(critter_ptr, target_tile, elevation or 0)
      ffi.C.ck_anim_end()
    end)
    return self
  end

  function builder:play(anim_id)
    table.insert(queue, function(critter_ptr)
      ffi.C.ck_anim_begin(critter_ptr, 0)
      ffi.C.ck_anim_play(critter_ptr, anim_id)
      ffi.C.ck_anim_end()
    end)
    return self
  end

  function builder:submit()
  end

  return builder
end

function Critter:_handle_map_update(current_ticks)
  -- 1: handle object's on:('map_update')
  if self.handlers['map_update'] then
    self.handlers['map_update'](self)
  end

  -- 2: if called but busy - return
  if self:is_busy() then return end

  -- 3: fifo queue
  if #self._action_queue > 0 then
    -- FIFO
    local next_action = table.remove(self._action_queue, 1)
    next_action(self.c_ptr)

    return
  end

  -- 4: exec behavior!
  if self.active_behavior and current_ticks >= self._next_behavior_tick then
    self._next_behavior_tick = current_ticks + self._behavior_interval

    self.active_behavior(self, current_ticks)
  end
end

return Critter
