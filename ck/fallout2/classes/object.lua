local Object = {}
Object.__index = Object

function Object.new(lua_id, sid, config)
  local self = setmetatable({}, Object)

  self.id = lua_id
  self.sid = sid
  self.name = config.name
  self.description = config.description
  self.handlers = {}

  Object.registry[lua_id] = self

  return self
end

function Object:on(event_name, callback)
  self.handlers[event_name] = callback
end

-- function Object:get_tile()
--     return fallout.obj_get_tile(self.id)
-- end

return Object
