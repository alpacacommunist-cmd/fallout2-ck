local assets = {}

local ffi = require('ffi')
local ck = require('ck')
local log = ck.log.new('assets.lua')

-- [mod_id] = { key = fid }
assets.registry = ck.registries.assets

function assets.resolve(asset_string, art_type)
  local mod_id = ck.tools.current_mod_id()
  art_type = art_type or 6

  -- lowercase path
  asset_string = string.lower(asset_string)

  -- append .frm if type isn't specified
  if not string.match(asset_string, "%.frm$") then
    asset_string = asset_string .. ".frm"
  end

  -- return cached asset if present
  if assets.registry[mod_id][asset_string] then
    return assets.registry[mod_id][asset_string]
  end

  local full_path = string.format("../mods/%s/assets/%s", mod_id, asset_string)

  local assigned_frm_id = ffi.C.ck_assets_register_path(full_path)
  if assigned_frm_id == -1 then
    log.error("Failed to register custom asset path (limit reached?): " .. full_path)
  end

  local fid = ffi.C.ck_ids_make_ck_fid(assigned_frm_id, art_type)
  assets.registry[mod_id][asset_string] = fid

  log.debug("Resolved '%s' -> FID: 0x%X (Path: %s)", asset_string, fid, full_path)

  return fid
end

return assets
