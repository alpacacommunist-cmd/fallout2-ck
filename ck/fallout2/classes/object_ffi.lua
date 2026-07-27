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

      bind = function(self)
        if self.lua_id ~= -1 then return objects.registry[self.lua_id] end

        self.lua_id  = ffi.C.ck_registry_modify_object(self.c_ptr)
        local object = objects.registry[self.lua_id]

        if not object then
          if self:is_critter() then
            object = Critter.new(self.lua_id, { name = ffi.string(self.name) }, ffi.string(self.name), self.mod_id)
          else
            object = Object.new(self.lua_id, {}, self.mod_id)
          end
        end

        return object
      end
    }

    return methods[key]
  end
})
