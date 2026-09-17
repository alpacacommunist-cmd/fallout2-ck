local assets = {}

local ck = require('ck')
local ffi = require('ffi')
local log = ck.log.new('assets.lua')

local path_to_fid_cache = {}

function assets.resolve(asset_string, art_type)
  art_type = art_type or 6

  if path_to_fid_cache[asset_string] then
    return path_to_fid_cache[asset_string]
  end

  local asset_mod_id, resource_path = asset_string:match("([^:]+):(.+)")
  -- assume local asset
  if not asset_mod_id or not resource_path then
    asset_mod_id  = ck.tools.current_mod_id()
    resource_path = asset_string
  end

  resource_path = string.lower(resource_path)

  if not string.match(resource_path, "%.frm$") then
    resource_path = resource_path .. ".frm"
  end

  local full_path = string.format("../mods/%s/assets/%s", asset_mod_id, resource_path)

  local assigned_frm_id = ffi.C.ck_assets_register_path(full_path)
  if assigned_frm_id == -1 then
    log.error("Failed to register custom asset path (limit reached?): " .. full_path)
  end

  local fid = ffi.C.ck_ids_make_ck_fid(assigned_frm_id, art_type)

  path_to_fid_cache[asset_string] = fid

  log.debug("Resolved '%s' -> FID: 0x%X (Path: %s)", asset_string, fid, full_path)

  return fid
end

return assets
