local skills  = require('ck.fallout2.objects.critters.skills')
local player  = require('ck.fallout2.player')
local monitor = require('ck.fallout2.monitor')

local function clamp(val, min_val, max_val)
  return math.max(min_val, math.min(max_val, val))
end

events.on('skill_used', function(skill_id, success_count, skill_bonus)
  local skill_name = skills.ID_MAP[skill_id]
  if not skill_name then
    log.warn("Unknown skill id: " .. tostring(skill_id))
    return
  end

  local current_val = player.skills[skill_name]

  if current_val >= 300 then return end

  local base_chance = (300 - current_val) / 5

  local difficulty_modifier = 3

  if skill_name == "outdoorsman" then
    if skill_bonus <= -10 then
      difficulty_modifier = -skill_bonus * 0.5
    else
      base_chance = base_chance * 0.15
      difficulty_modifier = -skill_bonus
    end
  elseif skill_name == "barter" then
    difficulty_modifier = math.min(15, skill_bonus / 100)
  else
    difficulty_modifier = -skill_bonus * 0.25
  end

  local success_modifier = 0
  if skill_name == "barter" then
    success_modifier = math.min(20, success_count / 20)
  else
    if success_count > 0 then
      success_modifier = 5
    end
  end

  local final_chance = base_chance + difficulty_modifier + success_modifier

  final_chance = clamp(final_chance, 1, 95)

  local roll = math.random(1, 100)

  if roll <= final_chance then
    player.add_skill(skill_name, 1)

    monitor.print(string.format("Skill %s is growing: %d%%", skill_name, current_val + 1))
  end
end)
