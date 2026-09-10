local registries = {
  events  = {},

  objects = {},

  timers  = {},
  timer_categories = { live = {} },

  spawn_counters = {}
}

local available_listeners = { 'onGameStart', 'onEngineReady', 'onModReload',
  'onDayPassed', 'onHourPassed', 'onTimeAdvance',
  'onBeforeGameLoad', 'onGameLoaded',
  'onDialogStart', 'skill_used', 'critter_killed',
  'map_enter', 'map_update'
}

function registries.init_mod(mod_id)
  table.insert(ck.active_mods, mod_id)

  registries.events[mod_id] = {}
  for index, listener in ipairs(available_listeners) do
    registries.events[mod_id][listener] = {}
  end

  registries.objects[mod_id] = {}

  registries.timers[mod_id]  = {}
  registries.timer_categories.live[mod_id] = {}

  registries.spawn_counters[mod_id] = 0
end

function registries.clear_mod(mod_id)
  for index = #ck.active_mods, 1, -1 do
    if ck.active_mods[index] == mod_id then
      table.remove(ck.active_mods, index)
      break
    end
  end

  registries.events[mod_id] = nil

  registries.objects[mod_id] = nil

  registries.timers[mod_id]  = nil
  registries.timer_categories.live[mod_id] = nil

  registries.spawn_counters[mod_id] = nil
end

return registries
