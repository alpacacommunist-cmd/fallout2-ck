local ffi = require("ffi")

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
      if    c_stat then return read_stat_fn(c_stat) end

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

local DERIVED_RULES = {
  {
    from = "endurance",
    to_id = stats.MAP.max_hp,
    multiplier = 2
  },
  {
    from = "strength",
    to_id = stats.MAP.carry_weight,
    multiplier = 25
  },
  -- { from = "endurance", to_id = stats.MAP.poison_resistance, multiplier = 5 },
  -- { from = "perception", to_id = stats.MAP.sequence,          multiplier = 2 },
}

local function recalculate_derived_bonuses(c_ptr, stats_table)
  for _, rule in ipairs(DERIVED_RULES) do
    ffi.C.ck_critter_set_bonus_stat(c_ptr, rule.to_id, 0)
  end

  for _, rule in ipairs(DERIVED_RULES) do
    local target_value = stats_table[rule.from]

    if target_value and target_value > 10 then
      local target_bonus = target_value - 10
      local bonus_to_add = target_bonus * rule.multiplier

      local current_bonus = ffi.C.ck_critter_get_bonus_stat(c_ptr, rule.to_id)

      ffi.C.ck_critter_set_bonus_stat(c_ptr, rule.to_id, current_bonus + bonus_to_add)
    end
  end
end

function stats.assign(c_ptr, stats_table)
  if not c_ptr or type(stats_table) ~= "table" then return false end

  local max_hp_id = stats.MAP.max_hp
  local hp_id     = stats.MAP.hp
  local stats_changed = false

  for stat_name, target_value in pairs(stats_table) do
    local stat_id = stats.MAP[stat_name]

    if stat_id and stat_id >= 0 and stat_id <= 6 then
      local current_base  = ffi.C.ck_critter_get_base_stat(c_ptr, stat_id)
      local current_bonus = ffi.C.ck_critter_get_bonus_stat(c_ptr, stat_id)
      local current_total = current_base + current_bonus

      if current_total ~= target_value then
        local target_base = target_value
        local target_bonus = 0

        if target_value > 10 then
          target_base = 10
          target_bonus = target_value - 10
        end

        ffi.C.ck_critter_set_base_stat(c_ptr, stat_id, target_base)
        ffi.C.ck_critter_set_bonus_stat(c_ptr, stat_id, target_bonus)

        stats_changed = true
      end
    end
  end

  recalculate_derived_bonuses(c_ptr, stats_table)

  if stats_table.max_hp then
    local current_max_hp = ffi.C.ck_critter_get_max_hp(c_ptr)

    if stats_table.max_hp > current_max_hp then
      local extra_bonus = stats_table.max_hp - current_max_hp

      local current_bonus = ffi.C.ck_critter_get_bonus_stat(c_ptr, max_hp_id)
      ffi.C.ck_critter_set_bonus_stat(c_ptr, max_hp_id, current_bonus + extra_bonus)

      stats_changed = true
    end
  end

  if stats_table.hp then
    ffi.C.ck_critter_set_current_hp(c_ptr, stats_table.hp)
  elseif stats_changed or stats_table.max_hp then
    ffi.C.ck_critter_set_full_hp(c_ptr)
  end

  return stats_changed
end

return stats
