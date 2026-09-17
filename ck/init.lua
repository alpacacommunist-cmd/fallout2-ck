local ck = {}

ck = {
  -- quick ref data table
  active_mods = {},
  -- flat list of loaded mods
  active_mods_list = {},
  -- mods sorted by type
  active_mods_by_type = {},
  -- list of loaded lib mods
  loaded_libs = {},

  language = 'english',

  log = require('ck.system.log'),
  tools = require('ck.system.mod_tools')
}

-- Mod gets this when requiring('ck')
ck.public = {
  language = ck.language,
  log = ck.log,
}

return ck
