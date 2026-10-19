local ffi = require('ffi')

local PERKS_MAP    = {} -- name -> int
local PERKS_ID_MAP = {} -- int -> name

local callback = ffi.cast("void (*)(const char*, int)", function(lua_name_ptr, value)
  local name = ffi.string(lua_name_ptr)

  PERKS_MAP[name]     = value
  PERKS_ID_MAP[value] = name
end)

ffi.C.ck_get_perks_metadata(callback)
callback:free()

local perks = { MAP = PERKS_MAP, ID_MAP = PERKS_ID_MAP }

function perks.create_proxy(read_perk_fn, add_rank_fn)
  local proxy = {}

  local perk_methods = {
    add_rank = function(self)
      if add_rank_fn then add_rank_fn(self.id) end
    end
  }

  setmetatable(proxy, {
    __index = function(_, key)
      local c_perk_id = perks.MAP[key]
      if not c_perk_id then return nil end

      return setmetatable({
        id = c_perk_id,
        name = key,
        rank = read_perk_fn(c_perk_id)
      }, {
        __index = perk_methods,
        __tostring = function(self) return tostring(self.rank) end
      })
    end
  })

  return proxy
end

return perks
