  -- typedef struct {
  --     void*     c_ptr;
  --     int       id;
  --     int       pid;
  --     int       sid;
  --     int       tile;
  --     int       elevation;
  --     int       flags;
  --     int       rotation;
  -- } CkObjectFFI;

local ffi    = require('ffi')
local Object = require('ck.fallout2.classes.object')

ffi.metatype("CkObjectFFI", {
  __index = {

    hijack = function(self)
      local current_mod = ffi.C.ck_get_current_mod_id()

      return Object.hijack_existing(self.c_ptr, self.pid, current_mod)
    end,

    on = function(self, event_name, callback)
      local managed_obj = self:hijack()

      return managed_obj:on(event_name, callback)
    end,

    is_critter = function(self)
      return bit.rshift(self.pid, 24) == 1
    end
  }
})
