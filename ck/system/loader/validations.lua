-- ck/fallout2/loader/validations.lua
local M = {}

function M.validateLocation(loc, manifest)
  if type(loc) ~= "table" then return false, "Loction must be a table" end

  -- name
  if type(loc.name) ~= "string" or loc.name:gsub("%s+", "") == "" then
    return false, "Location name is missing or is invalid (name)"
  end

  -- filename
  if type(loc.map_file) ~= "string" then
    return false, string.format("For location '%s' map_file has to be a string", loc.name)
  end

  if #loc.map_file < 1 or #loc.map_file > 8 then
    return false, string.format("For location '%s' filename '%s' has to be 1 to 8 symbols long", loc.name, loc.map_file)
  end

  -- worldmap coordinates
  if type(loc.world_pos) ~= "table" or #loc.world_pos < 2 then
    return false, string.format("For location '%s' world_pos must include {x, y}", loc.name)
  end
  if type(loc.world_pos[1]) ~= "number" or type(loc.world_pos[2]) ~= "number" then
    return false, string.format("For location '%s' world_pos coordinates must be integer", loc.name)
  end

  -- entrance
  if type(loc.entrance) ~= "table" then
    return false, string.format("For location '%s' entrance structure is missing", loc.name)
  end
  if type(loc.entrance.x) ~= "number" or type(loc.entrance.y) ~= "number" or type(loc.entrance.tile) ~= "number" then
    return false, string.format("For location '%s' in entrance x, y and tile must be integer", loc.name)
  end

  return true
end

return M
