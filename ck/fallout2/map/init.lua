-- ck/fallout2/map/init.lua
local ffi = require('ffi')
local ck = require('ck')
local assets = require('ck.fallout2.assets')

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
map.get_id            = ffi.C.ck_current_map_id
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

-- Search objects on a map
local _collection = {
  __index = {
    first_by_pid = function(results, target_pid)
      for _, obj in ipairs(results) do
        if obj.pid == target_pid then return obj end
      end
      return nil
    end
  }
}

local _search = function(tile, max_count, callback)
  max_count = max_count or 32
  local buffer = ffi.new("struct fallout_Object*[?]", max_count)

  local count = callback(tile, buffer, max_count)
  local results = {}

  local mod_id = ck.tools.current_mod_id()

  for index = 0, count - 1 do
    local raw_object = buffer[index]

    local existing_entry = ck.registries.objects[mod_id][raw_object.id]
    if existing_entry then
      table.insert(results, existing_entry)
    else
      table.insert(results, raw_object)
    end
  end

  return setmetatable(results, _collection)
end

function map.find_at_tile(tile, max_count)
  return _search(tile, max_count, ffi.C.ck_object_find_at_tile)
end

function map.find_by_pid(tile, max_count)
  return _search(tile, max_count, ffi.C.ck_object_find_by_pid)
end

function map.elevation()
  return ffi.C.ck_current_elevation()
end

return map
