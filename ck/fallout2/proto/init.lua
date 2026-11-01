local ffi = require('ffi')
local ck = require('ck')
local validations = require('ck.fallout2.proto.validations')

local log = ck.log.new('proto.lua')

local proto = {}

proto.PROC_NAMES = {
  [3]  = "description",
  [4]  = "pickup",
  [5]  = "drop",
  [6]  = "use",
  [11] = "talk",
  -- [12] = "critter", -- doesn't work bc gScriptsLists, using own on_map_update from ck_script.cc
  [13] = "combat",
  [14] = "damage",
  [18] = "destroy",
  [21] = "look_at",
  [22] = "timed",
  -- [23] = "map_update", -- doesn't work bc gScriptsLists, using own on_map_update from ck_script.cc
  [24] = "push"
}

proto.types = {
  item = 0,
  critter = 1,
}

-- { [mod_id] = { items = { [pid] = proto_instance }, critters = { [pid] = proto_instance } }
proto.registry = ck.registries.prototypes
-- { [mod_id] = { ["previous_tag"] = "new_tag" } }
proto.migrations = ck.registries.prototype_migrations
-- quickref map for tags
-- { mod_id = { items = {}, critters, = {} } ... }
proto.tags_registry = ck.registries.prototype_tags

local Proto = {}
Proto.__index = Proto

function Proto:new(source_pid, lua_tag, mod_id, object_type, config)
  local instance = setmetatable({}, self)

  if (not validations.base_params(source_pid, lua_tag, config)) then
    return nil
  end

  instance.object_type = object_type

  instance.mod_id = mod_id
  instance.source_pid = source_pid
  instance.lua_tag = lua_tag
  instance.name = config.name
  instance.description = config.description

  return instance
end

function proto.register_prototype(source_pid, lua_tag, config)
  local object_type = config.object_type or -1

  if object_type == -1 then
    logger.error("Unsupported object type: %d", config.object_type)
  end

  if object_type == proto.types.item then
    return proto.register_item_prototype(source_pid, lua_tag, config)
  end
end

function proto.rename(previous_name, new_name)
  local mod_id = ck.tools.current_mod_id()

  proto.migrations[mod_id][previous_name] = new_name
  log.info("[%s] Registered tag rename: %s -> %s", mod_id, previous_name, new_name)
end

proto.ProtoClass = Proto
proto.validations = validations

return proto
