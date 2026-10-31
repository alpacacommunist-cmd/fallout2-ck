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
-- quickref map for tags
-- { mod_id = { items = {}, critters, = {} } ... }
proto.tags_registry = ck.registries.prototype_tags

function proto.register_item_prototype(source_pid, lua_tag, config)
  if (not validations.register_item_params(source_pid, lua_tag, config)) then
    return nil
  end

  local mod_id = ck.tools.current_mod_id()

  if proto.tags_registry[mod_id].items[lua_tag] then
    log.error("Can't register proto: lua_tag is already taken: %s for mod_id: %s", lua_tag, mod_id)
    return nil
  end

  local ffi_data = ffi.new("ItemProtoFFI")

  ffi_data.lua_tag = lua_tag

  ffi_data.name = config.name
  ffi_data.description = config.description

  ffi_data.price  = config.price  or -1
  ffi_data.weight = config.weight or -1

  ffi_data.inv_fid    = config.inv_fid or -1
  ffi_data.ground_fid = config.ground_fid or -1

  ffi_data.usable = config.usable or false

  local pid = ffi.C.ck_item_proto_register(source_pid, ffi_data)
  if pid == -1 then
    log.error("Failed to define prototype: " .. tostring(lua_tag))
    return nil
  end

  local proto_instance = { lua_tag = lua_tag, pid = pid, handlers = {} }

  function proto_instance:on(event_name, callback)
    self.handlers[event_name] = callback

    return proto_instance
  end

  function proto_instance:_handle_proc(proc_id, fixed_param)
    local event_name = proto.PROC_NAMES[proc_id]
    if not event_name then return false end

    if self.handlers[event_name] then
      local result = self.handlers[event_name](self, fixed_param)

      if result ~= nil then return result end
    end
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

return proto
