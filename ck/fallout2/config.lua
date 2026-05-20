-- ck/fallout2/config.lua
local config = {
    overrides = {}
}

function config.set(key, value)
    config.overrides[key] = value
    print("[CK Config] Engine setting registered: " .. tostring(key) .. " = " .. tostring(value))
end

-- Use single declarative API
function ckOnGetConfig(key, defaultValue)
    local override = config.overrides[key]
    if override ~= nil then
        return override
    end

    return defaultValue
end

return config
