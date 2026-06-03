-- mods/temple_of_trials/outskirts.lua

local outskirts = {}

outskirts.anchorTile = 17290

outskirts.mountainTiles = {
  { fid = 1509, tile = 13893 },
  { fid = 1508, tile = 14896 },
  { fid = 1508, tile = 13494 }
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
