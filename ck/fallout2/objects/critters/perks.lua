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

function perks.create_proxy(read_perk_fn)
  local proxy = {}
  setmetatable(proxy, {
    __index = function(_, key)
      local c_perk = PERKS_MAP[key]
      if    c_perk then return read_perk_fn(c_perk) end

      return nil
    end
  })

  return proxy
end

return perks
