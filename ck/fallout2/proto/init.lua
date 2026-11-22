local ffi = require('ffi')
local ck = require('ck')
local utils = require('ck.system.utils')

local validations = require('ck.fallout2.proto.validations')

local log = ck.log.new('proto.lua')

local proto = {}

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

-- allows both string, int values
local function normalize_allowed_procs(raw_procs)
  -- normalized int flat list
  local normalized_procs = {}

  -- return unless raw_procs is a table and has entries
  if not raw_procs or not type(raw_procs) == "table" or next(raw_procs) == nil then
    return normalized_procs
  end

  for _index, proc_id in ipairs(raw_procs) do
    local proc = ck.common.script_proc_id_normalized(proc_id)
    if (not proc) then
      log.warning("unknown proc_id: %d, ignoring", proc_id)
    end

    normalized_procs[proc] = true
  end

  return normalized_procs
end

function Proto:new(source_pid, lua_tag, mod_id, object_type, config)
  local instance = setmetatable({}, self)

  if (not validations.base_params(source_pid, lua_tag, config)) then
    return nil
  end

  instance.object_type = object_type
  instance.allowed_procs = normalize_allowed_procs(config.allowed_procs)

  instance.mod_id = mod_id
  instance.source_pid = source_pid
  instance.lua_tag = lua_tag
  instance.name = config.name
  instance.description = config.description

  return instance
end

function Proto:has_allowed_procs()
  return next(self.allowed_procs) ~= nil
end

function proto.rename(previous_name, new_name)
  local mod_id = ck.common.current_mod_id()

  proto.migrations[mod_id][previous_name] = new_name
  log.info("[%s] Registered tag rename: %s -> %s", mod_id, previous_name, new_name)
end

proto.ProtoClass = Proto
proto.validations = validations

return proto
