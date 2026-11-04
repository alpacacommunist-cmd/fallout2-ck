local ck = require('ck')
local ffi = require('ffi')

local state = require('ck.fallout2.state')
local validations = require('ck.fallout2.objects.critters.validations')
local utils = require('ck.system.utils')

local CritterClass = require('ck.fallout2.classes.critter')
local CritterProto = require('ck.fallout2.proto.critter')

local log = ck.log.new('objects/critters.lua')

local critters = {}

-- Handles respawns (holds it's registry)
critters.respawns = require('ck.fallout2.objects.critters.respawns')

-- flat list of mod's spawned tags
-- used by state.db GC to clear out unused tags
critters.active_tags = ck.registries.critter_active_tags

-- Allocates critter prototype
-- (lives in a map context)
function critters.register_prototype(source_pid, lua_tag, config)
  local mod_id = ck.tools.current_mod_id()
  local proto_instance = CritterProto:new(source_pid, lua_tag, mod_id, config)

  if ck.registries.prototype_tags[mod_id].critters[proto_instance.lua_tag] then
    log.error("Can't register proto: lua_tag is already taken: %s for mod_id: %s", lua_tag, mod_id)
    return nil
  end

  return proto_instance:register()
end

-- Spawn/create
-- [debug] args tracer
local _ck_critter_spawn_traced = utils.trace("FFI:ck_critter_spawn", ffi.C.ck_critter_spawn)

-- internal register
local _register_validated = function(tag, mod_id, params)
  -- check if critter is dead (state.db check)
  local mod_map_state_db = state.ensure_mod_map_namespace(mod_id)
  local critter_state = mod_map_state_db.objects[tag]

  if critter_state and critter_state.hp <= 0 then
    log.info("[%s] critter %s is dead", mod_id, tag)

    if ck.registries.prototypes[mod_id].critters[params.pid] then
      log.info("[%s] updating critter corpse: %s", mod_id, tag)
      ffi.C.ck_critter_handle_dead_critter_spawn(critter_state.id, params.pid, critter_state.tile)
    end

    return nil
  end

  -- spawn params
  local ffi_params = ffi.new("CritterLuaSpawnParams", {
    tag = tag,
    elevation = params.elevation,
    script_index = params.script_index,
    team = params.team
  })

  --- args tracer function
  local critter = _ck_critter_spawn_traced(params.pid, params.tile, ffi_params)

  if critter == nil then
    log.error("Failed to register critter (FFI) (tag: %s)", tag)
    return nil
  end

  -- update active tags list
  critters.active_tags[mod_id][tag] = true

  return CritterClass.new(critter, params)
end

-- main register function
function critters.register(tag, params)
  -- allows tag to be specified through params.tag or as first argument
  if type(tag) == "table" and params == nil then
    params = tag
    tag = params.tag
    params.tag = nil
  end

  if not validations.base_params(params) then
    log.error("(register): Couldn't create critter (params validation failing)")
    return nil
  end

  local mod_id = ck.tools.current_mod_id()

  if not validations.tag_presence_and_uniqueness(mod_id, tag) then
    log.error("(register): Couldn't create critter (tag validation failing)")
    return nil
  end

  -- sets defaults
  params.elevation    = params.elevation or ffi.C.ck_current_elevation()
  params.script_index = params.script_index or -1
  params.team         = params.team or -1
  params.respawn      = params.respawn or nil

  return _register_validated(tag, mod_id, params)
end

-- allows optional tag (nil is replaced with autotag)
function critters.create(tag, params)
  -- allows tag to be specified through params.tag or as first argument
  if type(tag) == "table" and params == nil then
    params = tag
    tag = params.tag
    params.tag = nil
  end

  if not validations.base_params(params) then
    log.error("(create): Couldn't create critter (params validation failing)")
    return nil
  end

  -- generates autotag anchor_{tile}
  local mod_id = ck.tools.current_mod_id()

  if tag == nil then tag = string.format("anchor_%d", params.tile) end
  if not validations.tag_presence_and_uniqueness(mod_id, tag) then
    log.error("(create): Couldn't create critter (tag validation failing)")
    return nil
  end

  -- sets defaults
  params.elevation    = params.elevation or ffi.C.ck_current_elevation()
  params.script_index = params.script_index or -1
  params.team         = params.team or -1
  params.respawn      = params.respawn or nil

  return _register_validated(tag, mod_id, params)
end

-- PUBLIC
critters.public = {
  create = critters.create,
  register = critters.register,
  register_prototype = critters.register_prototype
}

return critters
