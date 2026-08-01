-- mods/temple_of_trials/outskirts.lua

local outskirts = {}

outskirts.mountain_tiles = {
  { fid = 0x040005E5, tile = 13893 },
  { fid = 0x040005E4, tile = 14896 },
  { fid = 0x040005E4, tile = 15498 },
  { fid = 0x040005E4, tile = 15098 },
  { fid = 0x040005E4, tile = 15899 },
  { fid = 0x040005E4, tile = 12890 }
}

outskirts.mountain_scenery = {
  -- left cliff
  { fid = 0x020002C0, tile = 12290 },
  { fid = 0x020002BF, tile = 12097 },
  { fid = 0x020002BE, tile = 13301 },
  { fid = 0x020002BD, tile = 14092 },
  { fid = 0x02000568, tile = 14705 },
  { fid = 0x020002BE, tile = 14908 },
  { fid = 0x020002C2, tile = 15303 },

  -- right cliff
  { fid = 0x020002BE, tile = 14458 },
  { fid = 0x020002C2, tile = 14853 }
}

outskirts.remove_blockers = {18308, 18509, 18108, 18709, 22322, 22522, 22723, 22923, 22924, 23124}
outskirts.create_blockers = {16708, 16906, 16907, 16909, 16910, 17106, 17110, 17306, 17311,
17312, 17506, 17507, 17512, 17707, 17713, 17913, 18113, 18114, 18314, 18514, 18712, 18713, 18911 }

return outskirts
