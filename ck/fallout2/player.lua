local ffi = require("ffi")
local utils = require('ck.system.utils')

local stats  = require("ck.fallout2.objects.critters.stats")
local skills = require('ck.fallout2.objects.critters.skills')
local perks  = require('ck.fallout2.objects.critters.perks')

-- stats
local stats_proxy    = stats.create_proxy(ffi.C.player_stat)
local pc_stats_proxy = stats.create_pc_proxy(ffi.C.player_pc_stat)

-- skills
local skills_proxy = skills.create_proxy(ffi.C.player_skill)

-- perks
local perks_proxy = perks.create_proxy(ffi.C.player_perk)

local player = {
  id = nil,
  stats   = stats_proxy,
  stat    = ffi.C.player_stat,
  pc_stat = ffi.C.player_pc_stat,

  skills    = skills_proxy,
  skill     = ffi.C.player_skill,

  perks = perks_proxy,
  perk  = ffi.C.player_perk
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
  if not skills.ID_MAP[skill] then return end

  ffi.C.player_add_skill(skill, value)
end

function player.set_skill(skill, value)
  if not skills.ID_MAP[skill] then return end

  ffi.C.player_set_skill(skill, value)
end

function player.set_base_stat(stat, value)
  if not stats.MAP[stat] then return end

  ffi.C.player_set_base_stat(stats.MAP[stat], value)
end

function player.pid()
  return ffi.C.player_get_pid()
end

return player
