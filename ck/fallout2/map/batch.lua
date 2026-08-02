-- ck/fallout2/map/batch.lua
local ffi = require("ffi")

local batch = {}

function batch.tiles(tiles)
  local count = #tiles
  if count == 0 then return end

  local tiles_array = ffi.new("CkFFITile[?]", count)

  for index = 1, count do
    local tile     = tiles[index]

    tiles_array[index - 1].tile = tile.tile
    if type(tile.fid) == "number" then
      tiles_array[index - 1].fid = tile.fid
    else
      tiles_array[index - 1].fid = -1
    end
  end

  ffi.C.ck_map_batch_tiles(tiles_array, count)
end

function batch.scenery(scenery)
  local count = #scenery
  if count == 0 then return end

  local scenery_array = ffi.new("CkFFIScenery[?]", count)

  for index = 1, count do
    local scenery     = scenery[index]

    scenery_array[index - 1].tile = scenery.tile
    if type(scenery.fid) == "number" then
      scenery_array[index - 1].fid = scenery.fid
    else
      scenery_array[index - 1].fid = -1
    end
  end

  ffi.C.ck_map_batch_scenery(scenery_array, count)
end

function batch.blockers(blockers)
  local count = #blockers
  if count == 0 then return end

  local blockers_array = ffi.new("CkFFIBlocker[?]", count)

  for index = 1, count do
    blockers_array[index - 1].tile = blockers[index]
  end

  ffi.C.ck_map_batch_blockers(blockers_array, count)
end

function batch.clear(objects)
  local count = #objects
  if count == 0 then return end

  local objects_array = ffi.new("CkFFIClear[?]", count)

  for index = 1, count do
    objects_array[index - 1].tile = objects[index]
  end

  ffi.C.ck_map_batch_clear(objects_array, count)
end

return batch
