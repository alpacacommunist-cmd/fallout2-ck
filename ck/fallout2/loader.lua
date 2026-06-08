-- ck/fallout2/loader.lua
local rendering = require('ck.fallout2.rendering')
local events = require('ck.fallout2.events')

-- extend path to include fallout2-ck/mods
-- package.path = package.path .. ";../mods/?.lua;../mods/?/init.lua"
package.path = package.path .. ";../?.lua;../?/init.lua"

print("[CK Loader] Initializing Mod Loader...")

-- Mod list
local active_mods = {
  "game_time_extender",
  "username",
  "temple_of_trials"
}

local reloadable_mods = {
  "temple_of_trials"
}

-- Loads mods
function ckInitializeMods()
  print("[CK Loader] Loading active modules...")

  for _, mod_folder in ipairs(active_mods) do
    print("[CK Loader] Booting: " .. mod_folder)

    local success, err = pcall(function()
      -- requires each mod to use single entry point as init.lua
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

  -- clear persistent rendering
  events.clear()
  rendering.clear()

  -- unload reloadable lua modules
  for _, mod_folder in ipairs(reloadable_mods) do
    local target_prefix = "mods." .. mod_folder

    for mod_name in pairs(package.loaded) do
      if mod_name:match("^" .. target_prefix) then
        package.loaded[mod_name] = nil
        print("[CK Loader] Unloaded: " .. mod_name)
      end
    end
  end

  -- reload mods
  for _, mod_folder in ipairs(reloadable_mods) do
    local target_prefix = "mods." .. mod_folder
    print("[CK Loader] Reloading: " .. mod_folder)

    local success, err = pcall(function()
      require(target_prefix .. ".init")
    end)

    if not success then
      print("[CK Loader] ERROR reloading mod '" .. mod_folder .. "': " .. tostring(err))
    end
  end

  -- re-fire map enter event
  events.emit("onMapEnter")

  print("[CK Loader] Reload complete!")
end
