local ffi = require("ffi")

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
  self.in_combat = false
  self.active_behavior = nil
  self.is_dead = self:hp() <= 0

  if (self.is_dead) then
    log.debug(string.format("critter %s is dead!", self.name))
  end

  if (not self.is_dead) then
    -- behivours
    self._is_moving = false
    self._action_queue   = {}
    self._next_behavior_tick = 0
    self._behavior_interval  = 20

    -- stats
    self._stats_proxy = stats.create_proxy(function(stat_id)
      local base  = ffi.C.ck_critter_get_base_stat(self.c_ptr, stat_id)
      local bonus = ffi.C.ck_critter_get_bonus_stat(self.c_ptr, stat_id)

      return base + bonus
    end)

    if config and config.stats then self.stats = config.stats end
  end

  return self
end

function Critter:__index(key)
  if key == "stats" then return self._stats_proxy end

  local val = rawget(Critter, key)
  if val ~= nil then return val end

  return Object[key]
end

function Critter:__newindex(key, value)
  if key == "stats" then
    self._stats_pending = value

    stats.assign(self.c_ptr, value)
  else
    rawset(self, key, value)
  end
end

function Critter:gender() return ffi.C.ck_critter_get_gender(self.c_ptr) end
function Critter:hp()     return ffi.C.ck_critter_get_hp(self.c_ptr) end
function Critter:max_hp() return ffi.C.ck_critter_get_max_hp(self.c_ptr) end
function Critter:set_hp(hp) return ffi.C.ck_critter_set_current_hp(self.c_ptr, hp) end

function Critter:set_behavior(behavior_fn, ...)
  if self.is_dead then return false end

  if type(behavior_fn) ~= "function" then
    log.error("is not a function: " .. tostring(behavior_fn))
  else
    self.active_behavior = behavior_fn(...)
  end

  return self
end

function Critter:is_busy()
  if self.is_dead then return true end

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

function Critter:_handle_proc(proc_id, fixed_param)
  -- check if proc is already handled in Object
  if Object._handle_proc(self, proc_id, fixed_param) then return true end

  if self.is_dead then return false end

  local event_name = Object.PROC_NAMES[proc_id]
  if not event_name then return false end

  if event_name == "combat" then
    log.info(string.format("combat npc: %d, fixed_param: %d", self.lua_id, fixed_param))

    if fixed_param == 5 then
      return false
    end

    if fixed_param == 4 then
      self.in_combat = true
      ffi.C.ck_critter_process_turn(self.c_ptr, self.lua_id)
    end

    return true

  elseif event_name == "talk" then
    if not dialogue.is_registered(self.lua_id) then return end

    dialogue.start(self.lua_id)
    self:clear_animations():emit('dialogue_finished')

    return true

  elseif event_name == "push" then
    return false
  end

  return false
end

function Critter:_handle_map_update(current_ticks)
  if self.is_dead then return false end
  if ffi.C.ck_in_combat() then return end

  -- 1: handle object's on:('map_update')
  if self.handlers['map_update'] then self.handlers['map_update'](self) end

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
