local assets = {}

local log = ck.log.new('assets.lua')
local ffi = require('ffi')

local path_to_fid_cache = {}

function assets.resolve(asset_string)
  -- asset_string "temple_of_trials:scenery/tree10"
  if path_to_fid_cache[asset_string] then
    return path_to_fid_cache[asset_string]
  end

  local mod_id, resource_path = asset_string:match("([^:]+):(.+)")
  if not mod_id or not resource_path then
    error("Invalid asset string format: " .. tostring(asset_string))
  end

  local full_path = string.format("../mods/%s/art/%s.frm", mod_id, resource_path)

  local assigned_frm_id = ffi.C.ck_assets_register_path(full_path)
  if assigned_frm_id == -1 then
    error("Failed to register custom asset path (limit reached?): " .. full_path)
  end

  local final_fid = ffi.C.ck_ids_make_ck_fid(assigned_frm_id)

  path_to_fid_cache[asset_string] = final_fid
  log.info("Resolved '%s' -> FID: 0x%X (Path: %s)", asset_string, final_fid, full_path)

  return final_fid
end

return assets
