-- mods/temple_of_trials/outskirts.lua

local outskirts = {}

outskirts.anchorTile = 17290

outskirts.mountainTiles = {
    -- === left big gap ===
  { fid = 1505, x = -530, y = -72 },
  { fid = 1506, x = -562, y = -70 },
  { fid = 1507, x = -595, y = -78 },
  { fid = 1508, x = -635, y = -74 },
  { fid = 1507, x = -685, y = -62 },


  { fid = 1505, x = -390, y = -170 },
  { fid = 1505, x = -385, y = -167 },
}

outskirts.mountainScenery = {
    -- left cliff
  { fid = 704, x = -500, y = -360 },
  { fid = 703, x = -725, y = -360 },
  { fid = 702, x = -725, y = -230 },
  { fid = 1384, x = -492, y = -250 },
  { fid = 702, x = -693, y = -55 },
  { fid = 701, x = -465, y = -155 },
  { fid = 1384, x = -690, y = -160 },
  { fid = 706, x = -500, y = -80 },


  -- right cliff
  { fid = 702, x = 430, y = -390 },
  { fid = 706, x = 630, y = -415 }
}

outskirts.walkableTiles = {
  { fid = 999, x = -464, y = 204 },
  { fid = 998, x = -464, y = 154 },
  { fid = 997, x = -455, y = 104 },
  { fid = 996, x = -455, y = 84 },
}

return outskirts
