local ffi = require("ffi")

local log = ck.log.new('classes/critter.lua')

local dialogue  = require('ck.fallout2.dialogue')
local monitor   = require('ck.fallout2.monitor')
local behaviors = require('ck.fallout2.objects.critters.behaviors')
local stats     = require('ck.fallout2.objects.critters.stats')

local Object = require("ck.fallout2.classes.object")
local critter_events = require('ck.fallout2.handlers.critter_events')

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

critter_events.attach(Critter)

function Critter:hp()     return ffi.C.ck_critter_get_hp(self.c_ptr) end
function Critter:max_hp() return ffi.C.ck_critter_get_max_hp(self.c_ptr) end
function Critter:set_hp(hp) return ffi.C.ck_critter_set_current_hp(self.c_ptr, hp) end

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

return Critter
