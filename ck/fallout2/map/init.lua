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

return map
