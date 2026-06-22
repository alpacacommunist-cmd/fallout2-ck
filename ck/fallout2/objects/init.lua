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

return objects
