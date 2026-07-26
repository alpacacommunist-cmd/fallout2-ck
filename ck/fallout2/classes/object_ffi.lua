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
local objects = require('ck.fallout2.objects')

ffi.metatype("CkObjectFFI", {
  __index = function(self, key)
    if self.lua_id ~= -1 and objects.registry[self.lua_id] then
      return objects.registry[self.lua_id][key]
    end

    local methods = {
      restore = function(self)
        return ffi.C.ck_registry_restore_modified_object(self.c_ptr)
      end,

      is_critter = function(self)
        return bit.rshift(self.pid, 24) == 1
      end,

      on = function(self, event_name, callback)
        if self.lua_id == -1 then
          self.lua_id = ffi.C.ck_registry_modify_object(self.c_ptr)
        end

        local mod_id = ffi.C.ck_get_current_mod_id()
        local object = objects.registry[self.lua_id]

        if not object then
          local config = { name = 'object_id: ' .. tostring(self.lua_id), description = 'test' }
          local Object  = require('ck.fallout2.classes.object')
          local Critter = require('ck.fallout2.classes.critter')

          if self:is_critter() then
            object = Critter.new(self.lua_id, config, config.name, mod_id)
          else
            object = Object.new(self.lua_id, config, mod_id)
          end
        end

        object:on(event_name, callback)

        return object
      end
    }

    return methods[key]
  end
})
