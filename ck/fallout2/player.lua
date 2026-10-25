local ffi = require("ffi")

local stats  = require("ck.fallout2.objects.critters.stats")
local skills = require('ck.fallout2.objects.critters.skills')
local perks  = require('ck.fallout2.objects.critters.perks')

-- stats
local stats_proxy  = stats.create_proxy(ffi.C.player_stat, ffi.C.player_set_base_stat)
local pc_stats_proxy = stats.create_pc_proxy(ffi.C.player_pc_stat)
-- skills
local skills_proxy = skills.create_proxy(
    ffi.C.player_skill,           -- read_total
    ffi.C.player_set_skill,       -- write_total
    ffi.C.player_get_skill_base   -- read_base
)
-- perks
local perks_proxy  = perks.create_proxy(ffi.C.player_perk, ffi.C.player_perk_add_rank)

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

function player.add_experience(xp)
  return ffi.C.player_give_experience(xp);
end

function player.pid()
  return ffi.C.player_get_pid()
end

function player.ptr()
  return ffi.C.player_get_ptr()
end

return player
