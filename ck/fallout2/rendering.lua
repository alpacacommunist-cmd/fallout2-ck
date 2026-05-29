-- ck/fallout2/rendering.lua

local rendering = {}

-- one frame draw
function rendering.drawScenery(fid, x, y)
  ck.rendering.draw_scenery(fid, x, y)
end

-- persistent draw
function rendering.addScenery(fid, tile, offsetX, offsetY)
  ck.rendering.add_scenery(fid, tile, offsetX, offsetY)
end

function rendering.addTile(fid, tile, offsetX, offsetY)
  ck.rendering.add_tile(fid, tile, offsetX, offsetY)
end

-- clear all
function rendering.clear()
  ck.rendering.clear()
end

function rendering.setBorders(left, right, top, bottom)
  ck.rendering.set_camera_borders(left, right, top, bottom)
end

return rendering
