local objects = {
  registry = {},

  PROC_NAMES = {
    [3]  = "description",
    [4]  = "pickup",
    [6]  = "use",
    [11] = "talk",
    -- [12] = "critter", -- doesn't work bc gScriptsLists, using own on_map_update from ck_script.cc
    [13] = "combat",
    [14] = "damage",
    [21] = "look_at",
    [22] = "timed",
    -- [23] = "map_update", -- doesn't work bc gScriptsLists, using own on_map_update from ck_script.cc
    [24] = "push"
  }
}

function objects.clear_for_mod(mod_name)
  for lua_id, object_instance in pairs(objects.registry) do
    if object_instance.mod_id == mod_name then
      objects.registry[lua_id] = nil
    end
  end
end

function objects.clear_registry()
  objects.registry = {}
end

return objects
