local ffi = require("ffi")

ffi.cdef[[
  void ck_get_stats_metadata(void (*callback)(const char* lua_name, int value));
  void ck_get_pc_stats_metadata(void (*callback)(const char* lua_name, int value));
]]

local STATS_MAP = {}
local PC_STATS_MAP = {}

local function register_fn(target_table)
  return function(lua_name_ptr, value)
    target_table[ffi.string(lua_name_ptr)] = value
  end
end

ffi.C.ck_get_stats_metadata(register_fn(STATS_MAP))
ffi.C.ck_get_pc_stats_metadata(register_fn(PC_STATS_MAP))

local stats = {
  MAP = STATS_MAP,
  PC_MAP = PC_STATS_MAP
}

function stats.create_proxy(read_stat_fn)
  local proxy = {}
  setmetatable(proxy, {
    __index = function(_, key)
      local c_stat = STATS_MAP[key]
      if c_stat then
        return read_stat_fn(c_stat)
      end
      return nil
    end
  })
  return proxy
end

function stats.create_pc_proxy(read_pc_stat_fn)
  local proxy = {}
  setmetatable(proxy, {
    __index = function(_, key)
      local c_pc_stat = PC_STATS_MAP[key]
      if c_pc_stat then
        return read_pc_stat_fn(c_pc_stat)
      end
      return nil
    end
  })
  return proxy
end

return stats
