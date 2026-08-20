-- ck/fallout2/map/init.lua
local ffi        = require("ffi")
local objects    = require('ck.fallout2.objects')
local object_ffi = require('ck.fallout2.classes.object_ffi')
local assets     = require('ck.fallout2.assets')

local map  = {
  geometry  = require('ck.fallout2.map.geometry'),
  tools     = require('ck.fallout2.map.tools'),
  exit_grid = require('ck.fallout2.map.exit_grid'),
  batch     = require('ck.fallout2.map.batch'),
  assets    = require('ck.fallout2.assets')
}

map.tools.init(map)

map.get_var           = ffi.C.ck_map_get_mvar
map.set_var           = ffi.C.ck_map_set_mvar
map.get_id            = ffi.C.ck_map_get_id
map.rendering_refresh = ffi.C.ck_rendering_refresh

map.physics = {
  create_blocker = ffi.C.ck_map_create_blocker_at,
  remove_blocker = ffi.C.ck_object_remove_at
}

-- Objects
map.objects = { floor = {}, roof = {}}

function map.objects.floor.create(pid, tile)
  ffi.C.ck_map_register_object(pid, tile)
end

function map.objects.roof.create(pid, tile, elevation_offset)
  elevation_offset = elevation_offset or 1
  ffi.C.ck_map_register_object_on_roof(pid, tile, elevation_offset)
end

-- Render
map.render = { floor = {}, roof = {} }

-- [Render] floors
function map.render.floor.tile(value, tile)
  local fid = (type(value) == "string") and assets.resolve(value) or value
  ffi.C.ck_map_add_tile_fid(fid, tile)
end

function map.render.floor.overlay(value, tile)
  local fid = (type(value) == "string") and assets.resolve(value) or value
  ffi.C.ck_map_add_scenery_fid(fid, tile)
end

-- [Render] roofs
function map.render.roof.tile(value, tile, roof_block_id)
  roof_block_id = roof_block_id or -1

  local fid = (type(value) == "string") and assets.resolve(value) or value
  ffi.C.ck_map_add_roof_tile_fid(fid, tile, roof_block_id)
end

function map.render.roof.overlay(value, tile, offset_y)
  local fid = (type(value) == "string") and assets.resolve(value) or value
  offset_y = offset_y or -96
  ffi.C.ck_map_add_roof_scenery_fid(fid, tile, offset_y)
end

-- Borders
function map.register_borders(map_id, config)
  assert(map_id,  "map_id is required!")
  assert(config.left,  "left edge hex coordinate is required!")
  assert(config.right,  "right edge hex coordinate is required!")
  assert(config.top,  "top edge hex coordinate is required!")
  assert(config.bottom,  "bottom edge hex coordinate is required!")

  local data = ffi.new("CkCameraBorders")

  data.left   = config.left
  data.right  = config.right
  data.top    = config.top
  data.bottom = config.bottom

  ffi.C.ck_map_set_camera_borders(map_id, data)
end

-- Search
function map.find_at_tile(tile)
  local max_count = 32
  local buffer    = ffi.new("CkObjectFFI[?]", max_count)

  local count = ffi.C.ck_object_find_at_tile(tile, buffer, max_count)

  local result = {}
  for index = 0, count - 1 do
    local copy = ffi.new("CkObjectFFI", buffer[index])

    table.insert(result, copy)
  end

  return setmetatable(result, ffi_object.collection)
end

function map.find_by_pid(pid, max_count)
  max_count    = max_count or 32
  local buffer = ffi.new("CkObjectFFI[?]", max_count)

  local count = ffi.C.ck_object_find_by_pid(pid, buffer, max_count)
  local results = {}

  for i = 0, count - 1 do
    local item = buffer[i]

    if item.lua_id ~= -1 and objects.registry[item.lua_id] then
      table.insert(results, objects.registry[item.lua_id])
    else
      table.insert(results, item)
    end
  end

  return results
end

function map.elevation()
  return ffi.C.ck_current_elevation()
end

return map
