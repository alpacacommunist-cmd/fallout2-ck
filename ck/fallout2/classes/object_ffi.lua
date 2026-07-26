  -- typedef struct {
  --     void*     c_ptr;
  --     int       id;
  --     int       pid;
  --     int       sid;
  --     int       tile;
  --     int       elevation;
  --     int       flags;
  --     int       rotation;
  --     int       lua_id;
  -- } CkObjectFFI;

local ffi     = require('ffi')
local Object  = require('ck.fallout2.classes.object')
local Critter = require('ck.fallout2.classes.critter')

ffi.metatype("CkObjectFFI", {
  __index = {

    on = function(self, event_name, callback)
      local id     = ffi.C.ck_registry_modify_object(self.c_ptr)
      local mod_id = ffi.C.ck_get_current_mod_id()

      local config = { name = 'object_id: ' .. tostring(self.id), description = 'test' }

      local object
      if self:is_critter() then
        object = Critter.new(id, config, config.name, mod_id)
      else
        object = Object.new(id, config, mod_id)
      end

      return object:on(event_name, callback)
    end,

    is_critter = function(self)
      return bit.rshift(self.pid, 24) == 1
    end
  }
})
