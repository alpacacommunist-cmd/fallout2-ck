-- mods/temple_of_trials/outskirts.lua

local outskirts = {}

outskirts.anchorTile = 17290

outskirts.mountainTiles = {
  { fid = 1508, tile = 13894 },
  { fid = 1507, tile = 14094 },
  { fid = 1505, tile = 14897 },
  { fid = 1505, tile = 13694 },
  { fid = 1509, tile = 13493 },
  { fid = 1510, tile = 14895 },
  { fid = 1510, tile = 15095 },
  { fid = 1510, tile = 14494 },
}

outskirts.mountainScenery = {
    -- left cliff
  { fid = 704, tile = 11090 },
  { fid = 703, tile = 10296 },
  { fid = 702, tile = 12102 },
  { fid = 1384, tile = 12494 },
  { fid = 702, tile = 14309 },
  { fid = 701, tile = 13698 },
  { fid = 1384, tile = 13106 },
  { fid = 706, tile = 14502 },

  -- right cliff
  { fid = 702, tile = 13661 },
  { fid = 706, tile = 13854 }
}

outskirts.walkableTiles = {
}

return outskirts
