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
]]

local C = ffi.C

local geometry = require('ck.fallout2.map.geometry')
local tools    = require('ck.fallout2.map.tools')
local assets   = require('ck.fallout2.assets')

local map      = {}

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

-- apply bindings to `tools`
tools._applyValue = function(value, objType, block, tile, mode)
  mode  = mode or "place"

  -- artId
  if type(value) == "number" then
    if objType == "tile" then
      map.add_tile_fid(value, tile)
    else
      map.create_object(value, tile)
    end
  else
    asset = assets.resolve(value)
    -- assets key
    -- tiles
    if objType == "tile" or (asset and asset.isTile) then
      map.add_tile_key(value, tile)

      return
    end
    -- scenery
    if mode == "draw" or not asset or asset.fid == -1 then
      map.add_scenery_fid(value, tile)
    else
      map.create_object_fid(asset.fid, tile)
    end
  end

  if block then map.create_blocker(tile) end
end

tools._onClear = function(tile)
  map.remove_blocker(tile)
end

map.geometry = geometry
map.tools    = tools

return map
