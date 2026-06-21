-- ck/fallout2/player.lua
local ffi = require("ffi")

ffi.cdef[[
  int player_stat(int stat);
  int player_pc_stat(int stat);

  typedef enum Stat {
    STAT_STRENGTH,
    STAT_PERCEPTION,
    STAT_ENDURANCE,
    STAT_CHARISMA,
    STAT_INTELLIGENCE,
    STAT_AGILITY,
    STAT_LUCK,
    STAT_MAXIMUM_HIT_POINTS,
    STAT_MAXIMUM_ACTION_POINTS,
    STAT_ARMOR_CLASS,
    STAT_UNARMED_DAMAGE,
    STAT_MELEE_DAMAGE,
    STAT_CARRY_WEIGHT,
    STAT_SEQUENCE,
    STAT_HEALING_RATE,
    STAT_CRITICAL_CHANCE,
    STAT_BETTER_CRITICALS,
    STAT_DAMAGE_THRESHOLD,
    STAT_DAMAGE_THRESHOLD_LASER,
    STAT_DAMAGE_THRESHOLD_FIRE,
    STAT_DAMAGE_THRESHOLD_PLASMA,
    STAT_DAMAGE_THRESHOLD_ELECTRICAL,
    STAT_DAMAGE_THRESHOLD_EMP,
    STAT_DAMAGE_THRESHOLD_EXPLOSION,
    STAT_DAMAGE_RESISTANCE,
    STAT_DAMAGE_RESISTANCE_LASER,
    STAT_DAMAGE_RESISTANCE_FIRE,
    STAT_DAMAGE_RESISTANCE_PLASMA,
    STAT_DAMAGE_RESISTANCE_ELECTRICAL,
    STAT_DAMAGE_RESISTANCE_EMP,
    STAT_DAMAGE_RESISTANCE_EXPLOSION,
    STAT_RADIATION_RESISTANCE,
    STAT_POISON_RESISTANCE,
    STAT_AGE,
    STAT_GENDER,
    STAT_CURRENT_HIT_POINTS,
    STAT_CURRENT_POISON_LEVEL,
    STAT_CURRENT_RADIATION_LEVEL,
    STAT_COUNT,

    PRIMARY_STAT_COUNT = 7,
    SPECIAL_STAT_COUNT = 33,
    SAVEABLE_STAT_COUNT = 35,
  } Stat;

  typedef enum PcStat {
      PC_STAT_UNSPENT_SKILL_POINTS,
      PC_STAT_LEVEL,
      PC_STAT_EXPERIENCE,
      PC_STAT_REPUTATION,
      PC_STAT_KARMA,
      PC_STAT_COUNT,
  } PcStat;
]]

local player = {
  id = nil

  stat = ffi.C.player_stat,
  pc_stat = ffi.C.player_pc_stat
}

local STATS_MAP = {
  strength     = ffi.C.STAT_STRENGTH,
  perception   = ffi.C.STAT_PERCEPTION,
  endurance    = ffi.C.STAT_ENDURANCE,
  charisma     = ffi.C.STAT_CHARISMA,
  intelligence = ffi.C.STAT_INTELLIGENCE,
  agility      = ffi.C.STAT_AGILITY,
  luck         = ffi.C.STAT_LUCK,
  hp           = ffi.C.STAT_CURRENT_HIT_POINTS,
  max_hp       = ffi.C.STAT_MAXIMUM_HIT_POINTS,
}

local PC_STATS_MAP = {
  unspent_skills = ffi.C.PC_STAT_UNSPENT_SKILL_POINTS,
  level          = ffi.C.PC_STAT_LEVEL,
  experience     = ffi.C.PC_STAT_EXPERIENCE,
  reputation     = ffi.C.PC_STAT_REPUTATION,
  karma          = ffi.C.PC_STAT_KARMA,
}

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

player.stats = stats_proxy

setmetatable(player, {
  __index = function(_, key)
    if key == "gender" then
      return player.stat(ffi.C.STAT_GENDER) == 0 and "male" or "female"
    elseif key == "level" then
      return pc_stats_proxy.level
    elseif key == "karma" then
      return pc_stats_proxy.karma
    end
    return nil
  end
})

return player
