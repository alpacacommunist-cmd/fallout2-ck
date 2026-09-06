local ffi = require("ffi")

local log = ck.log.new('objects/critters.lua')
local utils = require('ck.system.utils')

local CritterClass = require('ck.fallout2.classes.critter')
local ProtoClass   = require('ck.fallout2.classes.critter_proto')
local map          = require('ck.fallout2.map')
local stats        = require('ck.fallout2.objects.critters.stats')
local skills       = require('ck.fallout2.objects.critters.skills')

local critters = {}

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
  -- spawn params
  local spawn_params = ffi.new("CritterLuaSpawnParams", {
    tag = tag or nil,
    elevation = config.elevation or ffi.C.ck_current_elevation(),
    script_index = config.script_index or -1,
    team = config.team or -1
  })

  -- proto params (custom proto_name/description etc)
  local proto_name = not utils.is_blank(config.name) and config.name or nil
  local proto_description = not utils.is_blank(config.description) and config.description or nil
  local ai_packet  = not utils.is_blank(config.ai_packet) and config.ai_packet or nil
  local team_id = config.team or -1

  local proto_params = ffi.new("CritterLuaProtoParams", {
    name = proto_name, description = proto_description, ai_packet = ai_packet, team = team_id
  })

  local lua_id  = ck_critter_spawn_traced(pid, tile, spawn_params, proto_params)

  if lua_id == -1 then
    log.warn("Failed to register critter (FFI) (tag: %s)", lua_tag)
    return nil
  end

  if lua_id == -2 then
    log.debug("Critter %s is dead and has default prototype name/description", lua_tag)
    return nil
  end

  return CritterClass.new(lua_id, config)
end

function critters.create(pid, tile, config)
  local spawn_params = {}
  spawn_params.elevation    = config.elevation or 0
  spawn_params.script_index = config.script_index or -1
  spawn_params.team         = config.team or -1
  -- This is used for critters without explicitly specified tag
  -- Backend automatically generates spawn_{index} tag and skips custom prototype
  return critters.register(nil, pid, tile, spawn_params)
end

return critters
