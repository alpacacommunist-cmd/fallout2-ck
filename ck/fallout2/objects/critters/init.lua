local ffi = require("ffi")

local log = ck.log.new('objects/critters.lua')
local utils = require('ck.system.utils')

local CritterClass = require('ck.fallout2.classes.critter')
local map          = require('ck.fallout2.map')
local stats        = require('ck.fallout2.objects.critters.stats')
local skills       = require('ck.fallout2.objects.critters.skills')

local critters = {}

local ProtoClass = {}
ProtoClass.__index = ProtoClass

function ProtoClass.new(pid, name, description, ai_packet)
  local self = setmetatable({}, ProtoClass)
  self.pid   = pid
  self.name  = name
  self.description = description
  self.ai_packet = ai_packet

  self.c_ptr = ffi.C.ck_critter_get_proto_by_pid(self.pid)

  self.stats = stats.create_proxy(
    function(stat_id)        return ffi.C.ck_critter_proto_get_base_stat(self.c_ptr, stat_id) end,
    function(stat_id, value) ffi.C.ck_critter_proto_set_base_stat(self.c_ptr, stat_id, value) end
  )

  self.skills = skills.create_proxy(
    function(skill_id)        return ffi.C.ck_critter_proto_get_skill(self.c_ptr, skill_id) end,
    function(skill_id, value) ffi.C.ck_critter_proto_set_skill(self.c_ptr, skill_id, value) end
  )

  return self
end

function ProtoClass:set_stats(stats_table)
  if type(stats_table) ~= "table" then
    error("set_stats expects table, got: " .. type(stats_table))
  end

  for key, value in pairs(stats_table) do
    self.stats[key] = value
  end

  return self
end

function ProtoClass:set_skills(skills_table)
  if type(skills_table) ~= "table" then
    error("set_skills expects table, got: " .. type(skills_table))
  end

  for key, value in pairs(skills_table) do
    self.skills[key] = value
  end

  return self
end

function critters.allocate_prototype(pid, config)
  local proto_name = not utils.is_blank(config.name) and config.name or nil
  local proto_description = not utils.is_blank(config.description) and config.description or nil
  local ai_packet  = not utils.is_blank(config.ai_packet) and config.ai_packet or nil

  local proto_params = ffi.new("CritterLuaProtoParams", {
    name = proto_name,
    description = proto_description,
    ai_packet = ai_packet,
  })

  local allocated_pid = ffi.C.ck_critter_allocate_prototype(pid, proto_params)

  return ProtoClass.new(allocated_pid, proto_name, proto_description, ai_packet)
end

--- args tracer
local ck_critter_spawn_traced = utils.trace("FFI:ck_critter_spawn", ffi.C.ck_critter_spawn)

function critters.register(tag, pid, tile, config)
  config = config or {}
  config.elevation = config.elevation or ffi.C.ck_current_elevation()

  -- mod id
  local mod_id = ffi.string(ffi.C.ck_get_current_mod_id())

  -- params (custom proto_name/description etc)
  local proto_name = not utils.is_blank(config.name) and config.name or nil
  local proto_description = not utils.is_blank(config.description) and config.description or nil
  local ai_packet  = not utils.is_blank(config.ai_packet) and config.ai_packet or nil
  local team_id = config.team or -1

  local params = ffi.new("CritterLuaProtoParams", {
    name = proto_name,
    description = proto_description,
    ai_packet = ai_packet,
    team = team_id
  })

  -- returns { lua_id, lua_tag }
  local critter_data = ck_critter_spawn_traced(pid, tile, config.elevation, tag, params)
  local lua_tag = ffi.string(critter_data.lua_tag)

  if critter_data.lua_id == -1 then
    log.warn("Failed to register critter (FFI) (tag: %s)", lua_tag)
    return nil
  end

  if critter_data.lua_id == -2 then
    log.debug("Critter %s is dead and has default prototype name/description", lua_tag)
    return nil
  end

  return CritterClass.new(critter_data.lua_id, lua_tag, mod_id, config)
end

function critters.create(pid, tile, config)
  -- This is used for critters without explicitly specified tag
  -- Backend automatically generates spawn_{index} tag and skips custom prototype
  return critters.register(nil, pid, tile, config)
end

return critters
