local assets = {}

function assets.register(modId, basePath)
  ck.assets.register(modId, basePath)
end

function assets.resolve(key)
  return ck.assets.resolve(key)
end

return assets
