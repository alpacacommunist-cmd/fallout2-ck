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
      tiles_array[index - 1].key = nil
    else
      tiles_array[index - 1].fid = -1
      tiles_array[index - 1].key = tile.fid
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
      scenery_array[index - 1].key = nil
    else
      scenery_array[index - 1].fid = -1
      scenery_array[index - 1].key = scenery.fid
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
