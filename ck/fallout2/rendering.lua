-- ck/fallout2/rendering.lua
local ffi = require("ffi")

ck.rendering = {}

ck.rendering.refresh = ffi.C.ck_rendering_refresh
ck.rendering.clear   = ffi.C.ck_rendering_clear

local rendering = {
  refresh = ck.rendering.refresh,
  clear   = ck.rendering.clear
}

return rendering
