-- mods/temple_of_trials/outskirts.lua

local outskirts = {}

outskirts.new_exit_grid_rect = {}

outskirts.mountain_tiles = {
  { fid = 1509, tile = 13893 },
  { fid = 1508, tile = 14896 },
  { fid = 1508, tile = 15498 },
  { fid = 1508, tile = 15098 },
  { fid = 1508, tile = 15899 },
  { fid = 1508, tile = 12890 }
}

outskirts.mountain_scenery = {
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

outskirts.remove_blockers = {18308, 18509, 18108, 18709, 22322, 22522, 22723, 22923, 22924, 23124}
outskirts.create_blockers = {16708, 16906, 16907, 16909, 16910, 17106, 17110, 17306, 17311,
17312, 17506, 17507, 17512, 17707, 17713, 17913, 18113, 18114, 18314, 18514, 18712, 18713, 18911 }

return outskirts
