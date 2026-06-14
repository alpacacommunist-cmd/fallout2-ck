local assets = {}

function assets.register(mod_id, base_path)
  ck.assets.register(mod_id, base_path)
end

function assets.resolve(key)
  return ck.assets.resolve(key)
end

return assets
