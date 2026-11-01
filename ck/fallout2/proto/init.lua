local ffi = require('ffi')
local ck = require('ck')
local log = ck.log.new('proto.lua')
local validations = require('ck.fallout2.proto.validations')

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

-- { pid = { tag = '...', pid = .., handlers = {...} } }
proto.registry = {}
-- { [mod_id] = { ["previous_tag"] = "new_tag" } }
proto.migrations = ck.registries.prototype_migrations
-- quickref map for tags
-- { mod_id = { items = {}, critters, = {} } ... }
proto.tags_registry = ck.registries.prototype_tags

local Proto = {}
Proto.__index = Proto

function Proto:new(pid, lua_tag, mod_id, config)
  local instance = setmetatable({}, self)

  if (not validations.base_params(pid, lua_tag, config)) then
    return nil
  end

  instance.mod_id = mod_id
  instance.pid = pid
  instance.lua_tag = lua_tag
  instance.name = config.name
  instance.description = config.description

  return instance
end

local ItemProto = setmetatable({}, { __index = Proto })
ItemProto.__index = ItemProto

function ItemProto:new(pid, lua_tag, mod_id, config)
  local instance = Proto.new(self, pid, lua_tag, mod_id, config)

  instance.price = config.price or -1
  instance.weight = config.weight or -1
  instance.inv_fid = config.inv_fid or -1
  instance.ground_fid = config.ground_fid or -1
  instance.usable = config.usable or false

  instance.handlers = {}

  return instance
end

function ItemProto:to_ffi()
  local ffi_data = ffi.new("ItemProtoFFI")

  ffi_data.lua_tag = self.lua_tag
  ffi_data.name = self.name
  ffi_data.description = self.description

  ffi_data.price  = self.price
  ffi_data.weight = self.weight
  ffi_data.inv_fid    = self.inv_fid
  ffi_data.ground_fid = self.ground_fid
  ffi_data.usable = self.usable

  return ffi_data
end

function ItemProto:on(event_name, callback)
  self.handlers[event_name] = callback
  return self
end

function ItemProto:_handle_proc(proc_id, fixed_param)
  local event_name = proto.PROC_NAMES[proc_id]
  if not event_name then return false end

  if self.handlers[event_name] then
    local result = self.handlers[event_name](self, fixed_param)
    if result ~= nil then return result end
  end
end

function proto.register_item_prototype(source_pid, lua_tag, config)
  local mod_id = ck.tools.current_mod_id()
  local proto_instance = ItemProto:new(source_pid, lua_tag, mod_id, config)

  if proto.tags_registry[mod_id].items[proto_instance.lua_tag] then
    log.error("Can't register proto: lua_tag is already taken: %s for mod_id: %s", lua_tag, mod_id)
    return nil
  end

  local ffi_data = proto_instance:to_ffi()

  local pid = ffi.C.ck_item_proto_register(source_pid, ffi_data)
  if pid == -1 then
    log.error("Failed to define prototype: " .. tostring(lua_tag))
    return nil
  end

  proto.registry[pid] = proto_instance
  proto.tags_registry[mod_id].items[proto_instance.lua_tag] = true

  return proto_instance
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

return proto
