local ffi = require("ffi")

local utils = require('ck.system.utils')
local log = ck.log.new('assets.lua')

local assets = {}

ck.assets.register = ffi.C.ck_assets_register
ck.assets.resolve  = ffi.C.ck_assets_resolve

function assets.register(mod_id, base_path)
  ck.assets.register(mod_id, base_path)
end

function assets.resolve(key)
  local ck_asset = ck.assets.resolve(key)

  local asset = {
    file_path = ffi.string(ffi.C.ck_asset_file_path(key)),
    key = ffi.string(key),
    valid = ck_asset.valid,
    art_id = ck_asset.art_id,
    fid = ck_asset.fid,
    pid = ck_asset.pid,
    object_type = ck_asset.object_type,
    is_tile = ck_asset.is_tile,
    lookup_failed = ck_asset.lookup_failed
  }

  return asset
end

return assets
