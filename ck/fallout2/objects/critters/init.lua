local ffi = require("ffi")

local log = ck.log.new('objects/critters.lua')

local CritterClass = require('ck.fallout2.classes.critter')
local map          = require('ck.fallout2.map')
local state        = require('ck.fallout2.state')
local utils        = require('ck.system.utils')

local critters = {}

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

  local params = ffi.new("CritterLuaProtoParams", { proto_name, proto_description, ai_packet })

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
  return critters.register(nil, pid, tile, {})
end

return critters
