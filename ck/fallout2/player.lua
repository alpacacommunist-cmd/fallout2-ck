-- ck/fallout2/player.lua
local ffi = require("ffi")

ffi.cdef[[
  int player_stat(int stat);
  int player_pc_stat(int stat);

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

local stats_proxy = {}
local pc_stats_proxy = {}

setmetatable(stats_proxy, {
  __index = function(_, key)
    local c_stat = STATS_MAP[key]
    if c_stat then
      return ffi.C.player_stat(c_stat)
    end
    return nil
  end
})

setmetatable(pc_stats_proxy, {
  __index = function(_, key)
    local c_pc_stat = PC_STATS_MAP[key]
    if c_pc_stat then
      return ffi.C.player_pc_stat(c_pc_stat)
    end
    return nil
  end
})

local player = {
  id = nil,
  stats = stats_proxy,

  stat = ffi.C.player_stat,
  pc_stat = ffi.C.player_pc_stat
}

setmetatable(player, {
  __index = function(_, key)
    if key == "gender" then
      return player.stat(STATS_MAP.gender) == 0 and "male" or "female"
    elseif key == "level" then
      return pc_stats_proxy.level
    elseif key == "hp" then
      return stats_proxy.hp
    elseif key == "max_hp" then
      return stats_proxy.max_hp
    end
    return nil
  end
})

return player
