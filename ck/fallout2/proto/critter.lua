local ffi = require('ffi')
local ck = require('ck')

local utils = require('ck.system.utils')
local proto = require('ck.fallout2.proto')
local stats  = require('ck.fallout2.objects.critters.stats')
local skills = require('ck.fallout2.objects.critters.skills')

local log = ck.log.new('proto/critter.lua')

local ProtoClass = proto.ProtoClass
local CritterProto = setmetatable({}, { __index = ProtoClass })
CritterProto.__index = CritterProto

function CritterProto:new(source_pid, lua_tag, mod_id, config)
  local object_type = proto.types.critter
  local instance = ProtoClass.new(self, source_pid, lua_tag, mod_id, object_type, config)

  instance.ai_packet = (not utils.is_blank(config.ai_packet)) and config.ai_packet or nil

  if config.stats and not utils.table_is_empty(config.stats) then
    instance._stats_pending = config.stats
  end

  if config.skills and not utils.table_is_empty(config.skills) then
    instance._skills_pending = config.skills
  end

  return instance
end

function CritterProto:update_params()
  self.c_ptr = ffi.C.ck_critter_get_proto_by_pid(self.pid)

  self.stats = stats.create_proxy(
    function(stat_id)        return ffi.C.ck_critter_proto_get_base_stat(self.c_ptr, stat_id) end,
    function(stat_id, value) ffi.C.ck_critter_proto_set_base_stat(self.c_ptr, stat_id, value) end
  )
  self.skills = skills.create_proxy(
    function(skill_id)        return ffi.C.ck_critter_proto_get_skill(self.c_ptr, skill_id) end,
    function(skill_id, value) ffi.C.ck_critter_proto_set_skill(self.c_ptr, skill_id, value) end
  )

  if self._stats_pending then
    self:set_stats(self._stats_pending)
    self._stats_pending = nil
  end

  if self._skills_pending then
    self:set_skills(self._skills_pending)
    self._skills_pending = nil
  end

  return self
end

function CritterProto:to_ffi()
  local ffi_data = ffi.new("CritterProtoFFI")

  ffi_data.lua_tag = self.lua_tag
  ffi_data.name = self.name
  ffi_data.description = self.description

  ffi_data.ai_packet = self.ai_packet

  return ffi_data
end

function CritterProto:set_stats(stats_table)
  if type(stats_table) ~= "table" then
    log.error("set_stats expects table, got: %s (ignoring stats)", type(stats_table))
  end

  for key, value in pairs(stats_table) do
    self.stats[key] = value
  end

  return self
end

function CritterProto:set_skills(skills_table)
  if type(skills_table) ~= "table" then
    log.error("set_skills expects table, got: %s (ignoring skills)", type(skills_table))
  end

  for key, value in pairs(skills_table) do
    self.skills[key] = value
  end

  return self
end

function CritterProto:register()
  self.pid = ffi.C.ck_critter_allocate_prototype(self.source_pid, self:to_ffi())

  if self.pid == -1 then
    log.error("Failed to define prototype: %s", self.lua_tag)
    return nil
  end

  proto.registry[self.mod_id].critters[self.pid] = self
  proto.tags_registry[self.mod_id].critters[self.lua_tag] = true

  return self:update_params()
end

return CritterProto
