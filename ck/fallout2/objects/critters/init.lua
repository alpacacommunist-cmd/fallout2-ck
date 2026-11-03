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
  -- update active tags list
  critters.active_tags[mod_id][tag] = true

  -- check if critter is dead (state.db check)
  local mod_map_state_db = state.ensure_mod_map_namespace(mod_id)
  local critter_state = mod_map_state_db.objects[tag]

  if critter_state and critter_state.hp <= 0 then
    ffi.C.ck_critter_handle_dead_critter_spawn(critter_state.id, params.pid, critter_state.tile)

    log.info("[%s] critter %s is dead", mod_id, tag)
    return nil
  end

  -- if spawn_params.respawn then
  --   log.error("Can't use :respawn with critters.register, please use critters.create instead")
  --   return nil
  -- end

  -- _respawn_queue is set by timer callback (critters.respawns.append)
  -- if config._respawn_queue then
  --   tag = critters.respawns.generate_unique_tag(mod_id, config._respawn_queue)
  --
  --   local respawn_queue = critters.respawns.registry[mod_id][config._respawn_queue]
  --   respawn_queue.spawned_count = respawn_queue.spawned_count + 1
  -- end

  -- spawn params
  local ffi_params = ffi.new("CritterLuaSpawnParams", {
    tag = tag,
    elevation = params.elevation or ffi.C.ck_current_elevation(),
    script_index = params.script_index,
    team = params.team
  })

  --- args tracer function
  local lua_id = _ck_critter_spawn_traced(params.pid, params.tile, ffi_params)

  if lua_id == -1 then
    log.warning("Failed to register critter (FFI) (tag: %s)", tag)
    return nil
  end

  return CritterClass.new(lua_id, params)
end

-- main register function
function critters.register(tag, params)
  local mod_id = ck.tools.current_mod_id()

  if not validations.tag_presence_and_uniqueness(mod_id, tag) then
    log.error("(critters.register) can't register critter (tag: %s)", tag)
    return nil
  end

  params = (params and type(params) == "table") and params or {}
  if not validations.base_params(params) then
    log.error("(critters.register) can't register critter (tag: %s)", tag)
    return nil
  end

  validations.set_spawn_params_defaults(params)

  return _register_validated(tag, mod_id, params)
end

function critters.create(tag, params)
  local mod_id = ck.tools.current_mod_id()

  if not validations.tag_presence_and_uniqueness(mod_id, tag) then
    log.error("(critters.create) can't register critter (tag: %s)", tag)
    return nil
  end

  params = (params and type(params) == "table") and params or {}
  if not validations.base_params(params) then
    log.error("(critters.create) can't register critter (tag: %s)", tag)
    return nil
  end

  validations.set_spawn_params_defaults(params)

  -- modder registered respawn queue
  -- append callback and return
  -- if params.respawn then
  --   local mod_id = ck.tools.current_mod_id()
  --   return critters.respawns.append(mod_id, pid, tile, spawn_params, critters.register)
  -- end

  return _register_validated(tag, mod_id, params)
end

-- PUBLIC
critters.public = {
  create = critters.create,
  register = critters.register,
  register_prototype = critters.register_prototype
}

return critters
