local geometry = require('ck.fallout2.map.geometry')
local tools    = {}
local map_ref  = nil

function tools.init(map_instance)
  map_ref = map_instance
end

local function generic_brush(center_tile, radius, density, apply_fn)
  local tiles = geometry.tiles_in_radius(center_tile, radius)
  for _, tile in ipairs(tiles) do
    if tile ~= center_tile and math.random() <= density then
      apply_fn(tile)
    end
  end
end

local function generic_mask(anchor_tile, mask_table, mapping, apply_fn)
  local w = geometry.grid_width()
  local ax, ay = geometry.tile_to_xy(anchor_tile)

  for y, row in ipairs(mask_table) do
    local dy = y - 1
    for x = 1, #row do
      local char = row:sub(x, x)
      local dx = x - 1

      local tx = ax - dx - math.floor((ay + dy) / 2) + math.floor(ay / 2)
      local ty = ay + dy

      if tx >= 0 and tx < w and ty >= 0 and ty < w then
        local tile = geometry.xy_to_tile(tx, ty)

        if char == " " then
          map_ref.physics.remove_blocker(tile)
        elseif mapping[char] then
          apply_fn(tile, mapping[char])
        end
      end
    end
  end
end

tools.render = {}

function tools.render.brush(center_tile, radius, density, pool, config)
  config = config or {}
  generic_brush(center_tile, radius, density, function(tile)
    local asset = pool[math.random(#pool)]
    if config.type == "tile" then
      map_ref.render.tile(asset, tile)
    else
      map_ref.render.overlay(asset, tile)
    end
  end)
end

function tools.render.mask(anchor_tile, mask_table, mapping)
  generic_mask(anchor_tile, mask_table, mapping, function(tile, element)
    local pool = element.assets or element.fids
    if pool then
      local asset = pool[math.random(#pool)]
      if element.type == "tile" then
        map_ref.render.tile(asset, tile)
      else
        map_ref.render.overlay(asset, tile)
      end
    end

    if element.block then map_ref.physics.create_blocker(tile) end
  end)
end

tools.objects = {}

function tools.objects.brush(center_tile, radius, density, pid_pool)
  generic_brush(center_tile, radius, density, function(tile)
    local pid = pid_pool[math.random(#pid_pool)]
    map_ref.objects.create(pid, tile)
  end)
end

function tools.objects.mask(anchor_tile, mask_table, mapping)
  generic_mask(anchor_tile, mask_table, mapping, function(tile, element)
    local pool = element.pids
    if pool then
      local pid = pool[math.random(#pool)]
      map_ref.objects.create(pid, tile)
    end
  end)
end

return tools
