-- ck/fallout2/loader.lua

-- extend path to include fallout2-ck/mods
package.path = package.path .. ";../mods/?.lua;../mods/?/init.lua"

print("[CK Loader] Initializing Mod Loader...")

-- Mod list
local active_mods = {
    "game_time_extender",
    "username"
}

-- Loads mods
function ckInitializeMods()
    print("[CK Loader] Loading active modules...")

    for _, mod_folder in ipairs(active_mods) do
        print("[CK Loader] Booting: " .. mod_folder)

        local success, err = pcall(function()
            -- requires each mod to use single entry point as init.lua
            require(mod_folder .. ".init")
        end)

        if not success then
            print("[CK Loader] CRITICAL ERROR loading mod '" .. mod_folder .. "': " .. tostring(err))
        end
    end

    print("[CK Loader] All mods processed successfully!")
end
