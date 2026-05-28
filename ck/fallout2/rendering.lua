-- ck/fallout2/rendering.lua

local rendering = {}

-- one frame draw
function rendering.draw_scenery(fid, x, y)
  ck.rendering.draw_scenery(fid, x, y)
end

-- persistent draw
function rendering.add_scenery(fid, tile, offsetX, offsetY)
  ck.rendering.add_scenery(fid, tile, offsetX, offsetY)
end

function rendering.add_tile(fid, tile, offsetX, offsetY)
  ck.rendering.add_tile(fid, tile, offsetX, offsetY)
end

-- clear all
function rendering.clear()
  ck.rendering.clear()
end

return rendering
