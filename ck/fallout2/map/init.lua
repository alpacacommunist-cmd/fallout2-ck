-- ck/fallout2/map/init.lua
local ffi = require("ffi")

local C = ffi.C

local map  = {
  geometry  = require('ck.fallout2.map.geometry'),
  tools     = require('ck.fallout2.map.tools'),
  exit_grid = require('ck.fallout2.map.exit_grid'),
  batch     = require('ck.fallout2.map.batch'),
  assets    = require('ck.fallout2.assets')
}

map.tools.init(map)

map.get_var           = C.ck_map_get_mvar
map.set_var           = C.ck_map_set_mvar
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
map.rendering_refresh = C.ck_rendering_refresh

function map.register_object(value, tile)
  C.ck_map_register_object(value, tile)
end

function map.place(value, tile, config)
  config = config or {}
  local mode = config.mode or "place"

  local asset = (type(value) == "string") and map.assets.resolve(value) or nil
  local is_tile = (config.type == "tile") or (asset and asset.is_tile)

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
    elseif asset and asset.art_id then
      map.register_object(asset.art_id, tile)
    else
      map.add_scenery_key(value, tile)
    end
  end

  if config.block then map.create_blocker(tile) end
end

return map
