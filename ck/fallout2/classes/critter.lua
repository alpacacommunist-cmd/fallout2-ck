local ffi = require("ffi")

ffi.cdef[[
  void ck_critter_float_msg(int lua_id, const char* text, int msg_type);

  bool ck_in_combat();

  int  ck_anim_begin(void* ptr, int weapon_ready);
  int  ck_anim_move_to(void* ptr, int tile, int elevation);
  int  ck_anim_play(void* ptr, int anim_id);
  int  ck_anim_clear(void* ptr);
  int  ck_anim_end();
  bool ck_critter_is_busy(void* ptr);

  int ck_critter_get_base_stat(void* ptr, int stat_id);
  bool ck_critter_set_base_stat(void* ptr, int stat, int value);
  int ck_critter_get_bonus_stat(void* ptr, int stat);
  bool ck_critter_set_bonus_stat(void* ptr, int stat, int value);
  int player_stat(int stat);
  int player_pc_stat(int stat);

  int  ck_critter_get_hp(void* ptr);
  int  ck_critter_get_max_hp(void* ptr);
  int  ck_critter_set_current_hp(void* ptr, int target_hp);
  int  ck_critter_set_full_hp(void* ptr);
  bool ck_critter_kill(int lua_id);
]]

local log = ck.log.new('classes/critter.lua')

local dialogue  = require('ck.fallout2.dialogue')
local monitor   = require('ck.fallout2.monitor')
local behaviors = require('ck.fallout2.objects.critters.behaviors')
local stats     = require('ck.fallout2.objects.critters.stats')

local Object = require("ck.fallout2.classes.object")

local Critter = {}
setmetatable(Critter, { __index = Object })

function Critter.new(lua_id, config, tag, mod_id)
  local self = Object.new(lua_id, config, mod_id)
  setmetatable(self, Critter)

  self.tag = tag

  self.active_behavior = nil

  self._is_moving = false
  self._action_queue   = {}

  self._next_behavior_tick = 0
  self._behavior_interval  = 20

  self._stats_proxy = stats.create_proxy(function(stat_id)
    local base  = ffi.C.ck_critter_get_base_stat(self.c_ptr, stat_id)
    local bonus = ffi.C.ck_critter_get_bonus_stat(self.c_ptr, stat_id)
    return base + bonus
  end)

  if config and config.stats then self.stats = config.stats end

  self.in_combat = false

  return self
end

function Critter:__index(key)
  if key == "stats" then return self._stats_proxy end

  return Critter[key]
end

function Critter:__newindex(key, value)
  if key == "stats" then
    self._stats_pending = value

    stats.assign(self.c_ptr, value)
  else
    rawset(self, key, value)
  end
end

function Critter:hp()     return ffi.C.ck_critter_get_hp(self.c_ptr) end
function Critter:max_hp() return ffi.C.ck_critter_get_max_hp(self.c_ptr) end

function Critter:set_behavior(behavior_fn, ...)
  if type(behavior_fn) ~= "function" then
    log.error("is not a function: " .. tostring(behavior_fn))
  else
    self.active_behavior = behavior_fn(...)
  end

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
      return true
    end
  end

  if event_name == "look_at" then
    if (monitor and monitor.print and self.name) then
      monitor.print(self.name)

      return true
    end

  elseif event_name == "description" then
    if (monitor and monitor.print and self.description) then
      monitor.print(self.description)

      return true
    end

  elseif event_name == "destroy" then
    log.info('destroyed npc: ' .. tostring(self.id))
    ffi.C.ck_critter_kill(self.id)

    return true
  elseif event_name == "damage" then
    log.info('damage npc: ' .. tostring(self.id))

    return true


  elseif event_name == "talk" then
    if (dialogue and dialogue.start and dialogue.is_registered(self.id)) then
      dialogue.start(self.id)

      self:clear_animations()
        :emit('dialogue_finished')
      return true
    end
  end

  return false
end

function Critter:is_busy()
  return ffi.C.ck_critter_is_busy(self.c_ptr)
end

function Critter:clear_animations()
  ffi.C.ck_anim_clear(self.c_ptr)

  self._action_queue   = {}
  self._next_behavior_tick = 0
  self._behavior_interval  = 20
  self._is_moving = false

  return self
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
  if ffi.C.ck_in_combat() then return end

  -- 1: handle object's on:('map_update')
  if self.handlers['map_update'] then
    self.handlers['map_update'](self)
  end

  -- 2: if called but busy - return
  if self:is_busy() then
    return
  end

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
