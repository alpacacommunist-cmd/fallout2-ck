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
  --     const char* mod_id;
	--     const char* name;
  -- } CkObjectFFI;

local ffi     = require('ffi')
local Object  = require('ck.fallout2.classes.object')
local Critter = require('ck.fallout2.classes.critter')
local objects = require('ck.fallout2.objects')

object_ffi = {}

object_ffi.collection = {
  __index = {
    find_by_pid = function(results, target_pid)
      for _, obj in ipairs(results) do
        if obj.pid == target_pid then return obj end
      end
      return nil
    end
  }
}

function object_ffi:restore()
  return ffi.C.ck_registry_restore_modified_object(self.c_ptr)
end

function object_ffi:is_critter()
  return bit.rshift(self.pid, 24) == 1
end

function object_ffi:bind()
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

ffi.metatype("CkObjectFFI", {
  __index = function(self, key)
    if object_ffi[key] then return object_ffi[key] end

    -- ???
    -- if self.lua_id ~= -1 and objects.registry[self.lua_id] then
    --   return objects.registry[self.lua_id][key]
    -- end

    return nil
  end
})

return object_ffi
