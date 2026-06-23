-- ck/fallout2/i18n.lua
local LANGUAGE_MAP = {
  english = "en",
  russian = "ru",
  german  = "de",
  french  = "fr",
  spanish = "es",
  italian = "it",
}

local i18n = {
  language = "en",
  cache = {} -- mod_id -> { lang -> table }
}

local log = ck.log.new('CK I18n')

function i18n.register(mod_id, localeDir)
  i18n.cache[mod_id] = { dir = localeDir, loaded = {} }
end

local function load_locale(mod_id, lang)
  local mod = i18n.cache[mod_id]

  if not mod then
    log.error("Mod " .. tostring(mod_id) .. " is NOT registered!")
    return nil
  end

  if mod.loaded[lang] then return mod.loaded[lang] end

  local path = mod.dir .. '.' .. lang -- "mods.temple_of_trials.locale.ru"
  local ok, table = pcall(require, path)

  if ok then
    log.info("Successfully loaded locale from: " .. path)

    mod.loaded[lang] = table
    return table
  else
    log.error("Failed to require path: " .. path .. " | Error: " .. tostring(res))
  end

  return nil
end

local function get_nested_value(scope, key)
  if not scope then return nil end
  local current = scope

  for token in string.gmatch(key, "[^.]+") do
    if type(current) ~= "table" then return nil end
    current = current[token]
  end

  return current
end

function i18n.t(mod_id, key)
  local table = load_locale(mod_id, i18n.language)
  local value = get_nested_value(table, key)

  if value then return value end

  if i18n.language ~= "en" then
    table = load_locale(mod_id, "en")
    value = get_nested_value(table, key)
    if value then return value end
  end

  return key
end

function i18n.scoped(mod_id)
  return function(key)
    return i18n.t(mod_id, key)
  end
end

function i18n.bind(mod_id)
  local proxy = {}
  setmetatable(proxy, {
    __index = function(_, key) return i18n.t(mod_id, key) end
  })
  return proxy
end

function ckSetLanguage(lang)
  local low_lang = lang:lower()
  i18n.language = LANGUAGE_MAP[low_lang] or low_lang

  log.info("System language: " .. lang .. " -> Mapped to: " .. i18n.language)
end

return i18n
