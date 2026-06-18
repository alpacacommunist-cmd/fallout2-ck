-- ck/fallout2/map/init.lua
local ffi = require("ffi")

ffi.cdef[[
  int  ck_map_get_id();
  void ck_map_add_scenery_fid(int fid, int tile);
  void ck_map_add_scenery_key(const char* key, int tile);
  void ck_map_add_tile_fid(int fid, int tile);
  void ck_map_add_tile_key(const char* key, int tile);
  void ck_map_set_camera_borders(int left, int right, int top, int bottom);
  void ck_map_remove_blocker(int tile);
  void ck_map_create_blocker(int tile);
  void ck_map_create_object(int fid, int tile);
  void ck_map_create_object_fid(int fid, int tile);
  void ck_map_create_critter_pid(int pid, int tile, int sid);
  int  ck_map_register_object(int artId, int tile);
  int  ck_map_register_critter(int pid, int tile, const char* name, const char* description);

  void ck_landscape_destroy_pid_in_rect(int left, int right, int top, int bottom, int pid);
  void ck_landscape_destroy_exit_grid_in_rect(int left, int right, int top, int bottom);

  typedef struct { int tile; int fid; const char* key; } CkFFITile;
  typedef struct { int tile; int fid; const char* key; } CkFFIScenery;

  void ck_map_bulk_add_tiles(const CkFFITile* tiles, int count);
  void ck_map_bulk_add_scenery(const CkFFIScenery* sceneries, int count);
]]

local C = ffi.C

local map  = {
  geometry  = require('ck.fallout2.map.geometry'),
  tools     = require('ck.fallout2.map.tools'),
  exit_grid = require('ck.fallout2.map.exit_grid'),
  assets    = require('ck.fallout2.assets')
}

map.tools.init(map)

map.get_id            = C.ck_map_get_id
map.set_borders       = C.ck_map_set_camera_borders
map.add_tile_fid      = C.ck_map_add_tile_fid
map.add_tile_key      = C.ck_map_add_tile_key
map.add_scenery_fid   = C.ck_map_add_scenery_fid
map.add_scenery_key   = C.ck_map_add_scenery_key
map.create_object     = C.ck_map_create_object
map.create_object_fid = C.ck_map_create_object_fid
map.create_blocker    = C.ck_map_create_blocker
map.remove_blocker    = C.ck_map_remove_blocker
map.register_object   = C.ck_map_register_object
map.rendering_refresh = ck.rendering.refresh

map.register_critter = function(pid, tile, meta)
  local name = ""
  local description = ""

  if type(meta) == "table" then
    name        = meta.name or name
    description = meta.description or description
  end

  return C.ck_map_register_critter(pid, tile, name, description)
end

function map.place(value, tile, config)
  config = config or {}
  local mode = config.mode or "place"

  local asset = (type(value) == "string") and map.assets.resolve(value) or nil
  local is_tile = (config.type == "tile") or (asset and asset.isTile)

  -- render
  if mode == "draw" or is_tile then
    if type(value) == "number" then
      if is_tile then map.add_tile_fid(value, tile) else map.add_scenery_fid(value, tile) end
    else
      if is_tile then map.add_tile_key(value, tile) else map.add_scenery_key(value, tile) end
    end
  -- create_object
  else
    if type(value) == "number" then
      map.register_object(value, tile)
    elseif asset and asset.artId then
      map.register_object(asset.artId, tile)
    else
      map.add_scenery_key(value, tile)
    end
  end

  if config.block then map.create_blocker(tile) end
end

function map.add_tiles_bulk(tiles)
  local count = #tiles
  if count == 0 then return end

  local tiles_array = ffi.new("CkFFITile[?]", count)

  for index = 1, count do
    local tile     = tiles[index]
    local ffi_tile = tiles_array[index - 1]

    ffi_tile.tile = tile.tile

    if type(tile.fid) == "number" then
      ffi_tile.fid = tile.fid
      ffi_tile.key = nil
    else
      ffi_tile.fid = -1
      ffi_tile.key = tile.fid
    end
  end

  C.ck_map_bulk_add_tiles(tiles_array, count)
end

function map.add_scenery_bulk(scenery)
  local count = #scenery
  if count == 0 then return end

  local scenery_array = ffi.new("CkFFIScenery[?]", count)

  for index = 1, count do
    local scenery     = scenery[index]
    local ffi_scenery = scenery_array[index - 1]

    ffi_scenery.tile = scenery.tile
    if type(scenery.fid) == "number" then
      ffi_scenery.fid = scenery.fid
      ffi_scenery.key = nil
    else
      ffi_scenery.fid = -1
      ffi_scenery.key = scenery.fid
    end
  end

  C.ck_map_bulk_add_scenery(scenery_array, count)
end

return map
