-- ck/fallout2/rendering.lua
local ffi = require("ffi")

local C = ffi.C

ck.rendering.refresh = C.ck_rendering_refresh
ck.rendering.clear   = C.ck_rendering_clear

local rendering = {
  refresh = ck.rendering.refresh,
  clear   = ck.rendering.clear
}

return rendering
