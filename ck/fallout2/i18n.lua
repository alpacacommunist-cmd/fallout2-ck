-- ck/fallout2/i18n.lua
local i18n = {
  language = "english",
  cache = {} -- modId -> { lang -> table }
}

function i18n.register(modId, localeDir)
  i18n.cache[modId] = { dir = localeDir, loaded = {} }
end

local function load_locale(modId, lang)
  local mod = i18n.cache[modId]
  if not mod then return nil end

  if mod.loaded[lang] then return mod.loaded[lang] end

  local path = mod.dir .. '.' .. lang -- "mods.temple_of_trials.locale.ru"
  local ok, table = pcall(require, path)

  if ok then
    mod.loaded[lang] = table
    return table
  end

  return nil
end

-- t("temple_of_trials", "cave_name") -> contents
function i18n.t(modId, key)
  local table = load_locale(modId, i18n.language)
  if table and table[key] then return table[key] end

  if i18n.language ~= "english" then
    table = loadLocale(modId, "english")
    if table and table[key] then return table[key] end
  end

  return key
end

function ckSetLanguage(lang)
  i18n.language = lang:lower()
  print("[CK i18n] Language set to: " .. i18n.language)
end

return i18n
