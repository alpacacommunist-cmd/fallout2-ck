-- ck/fallout2/map/batch.lua
local ffi = require("ffi")

local batch = { floor = {}, roof = {} }

function batch.floor.tiles(tiles)
  if tiles == nil then return nil end

  local count = #tiles
  if count == 0 then return nil end

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

function batch.roof.tiles(tiles)
  if tiles == nil then return nil end

  local count = #tiles
  if count == 0 then return nil end

  local tiles_array = ffi.new("CkFFITile[?]", count)

  for index = 1, count do
    local tile = tiles[index]
    tiles_array[index - 1].tile = tile.tile
    tiles_array[index - 1].fid  = (type(tile.fid) == "number") and tile.fid or -1
    tiles_array[index - 1].roof_block_id = tile.roof_block_id or -1
  end

  ffi.C.ck_map_batch_roof_tiles(tiles_array, count)
end

function batch.scenery(scenery)
  if scenery == nil then return nil end

  local count = #scenery
  if count == 0 then return nil end

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
  if blockers == nil then return nil end

  local count = #blockers
  if count == 0 then return end

  local blockers_array = ffi.new("CkFFIBlocker[?]", count)

  for index = 1, count do
    blockers_array[index - 1].tile = blockers[index]
  end

  ffi.C.ck_map_batch_blockers(blockers_array, count)
end

function batch.clear(objects)
  if objects == nil then return nil end

  local count = #objects
  if count == 0 then return nil end

  local objects_array = ffi.new("CkFFIClear[?]", count)

  for index = 1, count do
    objects_array[index - 1].tile = objects[index]
  end

  ffi.C.ck_map_batch_clear(objects_array, count)
end

return batch
