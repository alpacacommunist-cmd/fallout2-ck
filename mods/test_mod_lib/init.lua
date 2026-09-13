local ffi = require("ffi")
local wall_utils = {}

local WALL_FID_NORTH = 124
local WALL_FID_WEST  = 125

function wall_utils.get_matching_fid(current_fid, direction)
  if direction == "north" then
    return WALL_FID_NORTH
  end
  return WALL_FID_WEST
end

function wall_utils.is_wall_blocking(hex_id)
  return false
end

return wall_utils
