-- ck/fallout2/loader.lua
local rendering = require('ck.fallout2.rendering')
local events    = require('ck.fallout2.events')
local assets    = require('ck.fallout2.assets')

package.path = package.path .. ";../?.lua;../?/init.lua"

print("[CK Loader] Initializing Mod Loader...")

local active_mods = {
  "game_time_extender",
  "username",
  "temple_of_trials"
}

local reloadable_mods = {
  "temple_of_trials"
}

local function loadManifest(mod_folder)
  local key = 'mods.' .. mod_folder .. '.mod'

  local ok, manifest = pcall(require, key)

  if not ok or type(manifest) ~= 'table' then
    print("[CK Loader] WARNING: no manifest for " .. mod_folder)
    return nil
  end

  return manifest
end

local function applyManifest(manifest)
  if not manifest then return end

  if manifest.assets then
    assets.register(manifest.id, manifest.assets)
  end
end

function ckInitializeMods()
  print("[CK Loader] Loading active modules...")

  for _, mod_folder in ipairs(active_mods) do
    print("[CK Loader] Booting: " .. mod_folder)

    -- manifest
    local manifest = loadManifest(mod_folder)
    applyManifest(manifest)

    -- init.lua
    local success, err = pcall(function()
      require('mods.' .. mod_folder .. ".init")
    end)

    if not success then
      print("[CK Loader] CRITICAL ERROR loading mod '" .. mod_folder .. "': " .. tostring(err))
    end
  end

  print("[CK Loader] All mods processed successfully!")
end

function ckReloadMods()
  print("[CK Loader] Reloading mods...")

  events.clear()
  rendering.clear()

  for _, mod_folder in ipairs(reloadable_mods) do
    local target_prefix = "mods." .. mod_folder

    for mod_name in pairs(package.loaded) do
      if mod_name:match("^" .. target_prefix) then
        package.loaded[mod_name] = nil
        print("[CK Loader] Unloaded: " .. mod_name)
      end
    end
  end

  for _, mod_folder in ipairs(reloadable_mods) do
    print("[CK Loader] Reloading: " .. mod_folder)

    local manifest = loadManifest(mod_folder)
    applyManifest(manifest)

    local success, err = pcall(function()
      require('mods.' .. mod_folder .. ".init")
    end)

    if not success then
      print("[CK Loader] ERROR reloading mod '" .. mod_folder .. "': " .. tostring(err))
    end
  end

  events.emit("onMapEnter")
  print("[CK Loader] Reload complete!")
end
