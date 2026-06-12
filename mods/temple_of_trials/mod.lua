-- mods/temple_of_trials/mod.lua
return {

  id      = "temple_of_trials",
  name    = "Temple of Trials",
  version = "0.1",

  assets = "mods/temple_of_trials/assets",
  maps   = "mods/temple_of_trials/maps",

  locations = {
    {
      name      = "Test Cave",
      map_file  = "TSTCV",
      music     = "07desert",
      world_pos = {220, 140},
      size      = "Small",
      entrance  = { x=130, y=410, tile=19275, rotation=0 }
    }
  }

}
