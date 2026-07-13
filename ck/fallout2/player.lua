local ffi = require("ffi")
local stats = require("ck.fallout2.objects.critters.stats")
local skills = require('ck.fallout2.objects.critters.skills')
local utils = require('ck.system.utils')

ffi.cdef[[
  int player_stat(int stat);
  int player_pc_stat(int stat);

  int player_skill(int skill);
  int player_skill_add(int skill, int value);
]]

local stats_proxy  = stats.create_proxy(ffi.C.player_stat)
local skills_proxy = skills.create_proxy(ffi.C.player_skill)
local pc_stats_proxy = stats.create_pc_proxy(ffi.C.player_pc_stat)

local player = {
  id = nil,
  stats   = stats_proxy,
  stat    = ffi.C.player_stat,
  pc_stat = ffi.C.player_pc_stat,

  skills    = skills_proxy,
  skill     = ffi.C.player_skill,
  skill_add = ffi.C.player_skill_add
}

setmetatable(player, {
  __index = function(_, key)
    if key == "gender" then
      return player.stat(stats.MAP.gender) == 0 and "male" or "female"
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

function player.add_skill(skill, value)
  if not skills.MAP[skill] then return end

  ffi.C.player_skill_add(skills.MAP[skill], value)
end

return player
