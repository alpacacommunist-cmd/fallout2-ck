local ffi = require("ffi")

local log = ck.log.new('objects/critters.lua')

local CritterClass = require('ck.fallout2.classes.critter')
local map          = require('ck.fallout2.map')
local state        = require('ck.fallout2.state')
local utils        = require('ck.system.utils')

local critters = {}

function critters.register(tag, pid, tile, config)
  config = config or {}
  config.elevation = config.elevation or ffi.C.ck_current_elevation()

  -- mod id
  local mod_id = ffi.string(ffi.C.ck_get_current_mod_id())

  -- params (custom proto_name/description etc)
  local proto_name = not utils.is_blank(config.name) and config.name or nil
  local proto_description = not utils.is_blank(config.description) and config.description or nil

  local params = ffi.new("CritterLuaProtoParams", { proto_name, proto_description })

  -- returns { lua_id, lua_tag }
  local critter_data = ffi.C.ck_critter_spawn(pid, tile, config.elevation, tag, params);

  if critter_data.lua_id == -1 then
    log.error("Failed to register critter (FFI)!")
    return nil
  end

  local lua_tag = ffi.string(critter_data.lua_tag)
  return CritterClass.new(critter_data.lua_id, config, lua_tag, mod_id);
end

function critters.create(pid, tile, config)
  -- This is used for critters without explicitly specified tag
  -- Backend automatically generates spawn_{index} tag and skips custom prototype
  return critters.register(nil, pid, tile, config)
end

return critters
