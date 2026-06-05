-- ck/fallout2/rendering.lua

local rendering = {}

-- one frame draw
function rendering.drawScenery(fid, x, y)
  ck.rendering.draw_scenery(fid, x, y)
end

-- clear all
function rendering.clear()
  ck.rendering.clear()
end

return rendering
