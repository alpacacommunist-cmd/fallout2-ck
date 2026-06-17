-- mods/temple_of_trials/outskirts.lua

local outskirts = {}

outskirts.new_exit_grid = { 23128, 23328, 23329, 23528, 23529, 23728, 23928 }
outskirts.new_exit_marker_tile = 23128

outskirts.mountainTiles = {
  { fid = 1509, tile = 13893 },
  { fid = 1508, tile = 14896 },
  { fid = 1508, tile = 15498 },
  { fid = 1508, tile = 15098 },
  { fid = 1508, tile = 15899 },
  { fid = 1508, tile = 12890 }
}

outskirts.mountainScenery = {
    -- left cliff
  { fid = 704, tile = 12290 },
  { fid = 703, tile = 12097 },
  { fid = 702, tile = 13301 },
  { fid = 701, tile = 14092 },
  { fid = 1384, tile = 14705 },
  { fid = 702, tile = 14908 },
  { fid = 706, tile = 15303 },

  -- right cliff
  { fid = 702, tile = 14458 },
  { fid = 706, tile = 14853 }
}

outskirts.removeBlockers = {18308, 18509, 18108, 18709}
outskirts.createBlockers = {16708, 16906, 16907, 16909, 16910, 17106, 17110, 17306, 17311,
17312, 17506, 17507, 17512, 17707, 17713, 17913, 18113, 18114, 18314, 18514, 18712, 18713, 18911 }

return outskirts
