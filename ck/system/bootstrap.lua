-- bootstrap.lua
local system_allowed = true

local function system_searcher(module_name)
  if not system_allowed then return nil end

  local sub = module_name:match("^ck%.system%.(.+)")
  if not sub then return nil end

  local rel_path = sub:gsub("%.", "/")
  local path = "../ck/system/" .. rel_path .. ".lua"

  local file = io.open(path, "r")
  if not file then
    path = "../ck/system/" .. rel_path .. "/init.lua"
    file = io.open(path, "r")
  end

  if not file then return nil end
  file:close()

  local loader_func, err = loadfile(path)
  if not loader_func then
    error("[CK System] Failed to load '" .. module_name .. "': " .. err)
  end

  return loader_func
end

table.insert(package.searchers or package.loaders, 1, system_searcher)

-- public api
package.path = package.path .. ";../?.lua;../?/init.lua"

ck.log = require('ck.system.log')

local loader = require('ck.system.loader')
local log    = ck.log.new("CK Bootstrap")

log.info("Bootstrapping Construction Kit...")

local active_mods = {
  "game_time_extender",
  "arroyo_expanded",
  "temple_of_trials"
}

system_allowed = false

for index, searcher in ipairs(package.searchers or package.loaders) do
  if searcher == system_searcher then
    table.remove(package.searchers or package.loaders, index)
    break
  end
end

for _, mod_folder in ipairs(active_mods) do loader.load_and_init_mod(mod_folder) end

log.info("Bootstrap complete!")
