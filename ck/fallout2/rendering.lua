-- ck/fallout2/rendering.lua
local ffi = require("ffi")

local rendering = {
  refresh = ffi.C.ck_rendering_refresh,
  clear   = ffi.C.ck_rendering_clear
}

return rendering
