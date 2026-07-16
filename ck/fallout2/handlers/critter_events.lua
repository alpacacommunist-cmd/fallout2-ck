local ffi = require("ffi")
local Object = require("ck.fallout2.classes.object")

local critter_events = {}

function critter_events.attach(Critter)

  function Critter:_handle_proc(proc_id, fixed_param)
    local event_name = Object.PROC_NAMES[proc_id]
    if not event_name then return false end

    if self.handlers[event_name] then
      if self.handlers[event_name](self) ~= false then return true end
    end

    if event_name == "look_at" then
      if (monitor and monitor.print and self.name) then
        monitor.print(self.name)

        return true
      end

    elseif event_name == "description" then
      if (monitor and monitor.print and self.description) then
        monitor.print(self.description)

        return true
      end

    elseif event_name == "destroy" then
      log.info('destroyed npc: ' .. tostring(self.id))
      ffi.C.ck_critter_kill(self.id)

      return true
    elseif event_name == "damage" then
      log.info('damage npc: ' .. tostring(self.id))

      return true
    elseif event_name == "combat" then
      log.info(string.format("combat npc: %d, fixed_param: %d", self.id, fixed_param))

      if fixed_param == 5 then
        return false
      end

      if fixed_param == 4 then
        self.in_combat = true
        ffi.C.ck_critter_process_turn(self.c_ptr, self.id)
      end

      return true

    elseif event_name == "talk" then
      if not (dialogue and dialogue.start and dialogue.is_registered(self.id)) then return end

      dialogue.start(self.id)
      self:clear_animations():emit('dialogue_finished')

      return true

    elseif event_name == "push" then
      return true
    end

    return false
  end

  function Critter:_handle_map_update(current_ticks)
    if ffi.C.ck_in_combat() then return end

    -- 1: handle object's on:('map_update')
    if self.handlers['map_update'] then self.handlers['map_update'](self) end

    -- 2: if called but busy - return
    if self:is_busy() then return end

    -- 3: fifo queue
    if #self._action_queue > 0 then
      -- FIFO
      local next_action = table.remove(self._action_queue, 1)
      next_action(self.c_ptr)

      return
    end

    -- 4: exec behavior!
    if self.active_behavior and current_ticks >= self._next_behavior_tick then
      self._next_behavior_tick = current_ticks + self._behavior_interval

      self.active_behavior(self, current_ticks)
    end
  end

end


return critter_events
