local ffi   = require('ffi')
local ck    = require('ck')
local utils = require('ck.system.utils')

local log   = ck.log.new('classes/critter.lua')

local dialogs   = require('ck.fallout2.dialogs')
local monitor   = require('ck.fallout2.monitor')
local behaviors = require('ck.fallout2.objects.critters.behaviors')
local stats     = require('ck.fallout2.objects.critters.stats')
local skills    = require('ck.fallout2.objects.critters.skills')

local Object = require("ck.fallout2.classes.object")

local Critter = {}
Critter.__index = Critter

setmetatable(Critter, { __index = Object })

function Critter.new(fallout_Object, config)
  local self = Object.new(fallout_Object)
  setmetatable(self, Critter)

  self.has_custom_prototype = ffi.C.ck_critter_has_custom_prototype(self.raw)
  if self.has_custom_prototype then
    self.proto = ck.registries.prototypes[self.mod_id].critters[self.pid]
  end

  self.in_combat       = false
  self.active_behavior = nil

  -- behivours
  self._next_behavior_tick = 0
  self._behavior_interval  = 20

  -- stats/skills (critterGetStat, critterGetSkill proxies)
  self.stats = stats.create_proxy(function(stat_id)
    local base  = ffi.C.ck_critter_get_base_stat(self.raw, stat_id)
    local bonus = ffi.C.ck_critter_get_bonus_stat(self.raw, stat_id)

    return base + bonus
  end)

  self.skills = skills.create_proxy(function(skill_id)
    return ffi.C.ck_critter_get_skill(self.raw, skill_id)
  end)

  return self
end

-- TODO: supposed to set bonus stats
function Critter:set_stats(stats_table)
  if stats_table == nil or utils.table_is_empty(stats_table) then return nil end

  stats.assign(self.raw, stats_table)
end

function Critter:set_skills(skills_table)
  if skills == nil or utils.table_is_empty(skills_table) then return nil end

  for skill_id, skill_value in pairs(skills_table) do
    ffi.C.ck_critter_set_skill(self.raw, skills.MAP[skill_id], skill_value)
  end
end

function Critter:gender() return ffi.C.ck_critter_get_gender(self.raw) end
function Critter:hp()     return ffi.C.ck_critter_get_hp(self.raw) end
function Critter:max_hp() return ffi.C.ck_critter_get_max_hp(self.raw) end
function Critter:set_hp(hp) return ffi.C.ck_critter_set_current_hp(self.raw, hp) end

function Critter:set_behavior(behavior_fn, ...)
  if type(behavior_fn) ~= "function" then
    log.error("is not a function: " .. tostring(behavior_fn))
  else
    -- disable default fallout2-ce map update procs
    -- and switch to lua map updates (doesn't require SID, dispatched directly)
    if not self:has_lua_script_index() then
      ffi.C.ck_script_disable_map_updates_for_object(self.raw)
    end

    self:clear_animations()
    self.active_behavior = behavior_fn(...)
  end

  return self
end

function Critter:is_busy()
  return ffi.C.ck_critter_is_busy(self.raw)
end

function Critter:clear_animations()
  ffi.C.ck_critter_anim_clear(self.raw)
  return self
end

function Critter:take_out_weapon()
  ffi.C.ck_anim_begin(self.raw, 0)
  ffi.C.ck_take_out_weapon(self.raw)
  ffi.C.ck_anim_end()
end

function Critter:animate(request_options)
  local steps = {}
  local critter = self
  local opts = request_options or 0

  local builder = {}

  -- delay: nil / -1 = wait prev step, 0 = parallel, >0 = delay (ticks)
  function builder:walk_to(target_tile, elevation, delay)
    table.insert(steps, {
      type = "walk",
      tile = target_tile,
      elev = elevation or critter.elevation or 0,
      delay = delay or -1
    })
    return self
  end

  function builder:run_to(target_tile, elevation, delay)
    table.insert(steps, {
      type = "run",
      tile = target_tile,
      elev = elevation or critter.elevation or 0,
      delay = delay or -1
    })
    return self
  end

  function builder:face_to(target_tile)
    table.insert(steps, {
      type = "face",
      tile = target_tile
    })
    return self
  end

  function builder:play(anim_id, delay)
    table.insert(steps, { type = "anim", id = anim_id, delay = delay or -1 })
    return self
  end

  function builder:submit()
    if #steps == 0 then return true end
    if ffi.C.ck_anim_begin(critter.raw, opts) ~= 0 then return false end

    for _, step in ipairs(steps) do
      local rc = -1
      if step.type == "walk" then
        rc = ffi.C.ck_critter_anim_move_to(critter.raw, step.tile, step.elev, step.delay)
      elseif step.type == "run" then
        rc = ffi.C.ck_critter_anim_run_to(critter.raw, step.tile, step.elev, step.delay)
      elseif step.type == "face" then
        rc = ffi.C.ck_critter_anim_face_to(critter.raw, step.tile)
      elseif step.type == "anim" then
        rc = ffi.C.ck_critter_anim_play(critter.raw, step.id, step.delay)
      end

      if rc ~= 0 then return false end
    end

    return ffi.C.ck_anim_end() == 0
  end

  return builder
end

local proc_handlers = {
  combat = function(self, fixed_param)
    log.info(string.format("combat npc: %d, fixed_param: %d", self.id, fixed_param))

    return false
  end,

  talk = function(self, fixed_param)
    if not dialogs.is_registered(self.id) then return end

    dialogs.start(self.id)
    self:clear_animations():emit('dialog_finished')

    return true
  end,

  push = function(self, fixed_param)
    return false
  end
}

function Critter:_handle_proc(proc_id, fixed_param)
  local event_name = Object.PROC_NAMES[proc_id]
  if not event_name then return false end

  -- check if proc is handled in Object
  local result = Object._handle_proc(self, proc_id, fixed_param)
  if result ~= nil then return result end

  -- defaults
  local default_handler = proc_handlers[event_name]
  if default_handler then
    local result = default_handler(self, fixed_param)
    if result ~= nil then return result end
  end

  return nil
end

function Critter:_handle_map_update(current_ticks)
  if ffi.C.ck_in_combat() then return end

  -- 1: handle object's on:('map_update')
  if self.handlers['map_update'] then self.handlers['map_update'](self) end

  -- 2: if called but busy - return
  if self:is_busy() then return end

  -- 4: exec behavior!
  if self.active_behavior and current_ticks >= self._next_behavior_tick then
    self._next_behavior_tick = current_ticks + self._behavior_interval

    self.active_behavior(self, current_ticks)
  end
end

return Critter

