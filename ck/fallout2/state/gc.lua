local ck = require('ck')
local registries = require('ck.system.registries')

local log = ck.log.new('state/gc.lua')

local gc = {}

-- Garbage Collection ✨
-- mod specific
function gc.purge_mod_namespace(mod_id, mod_map_db, ctx)
  for key in pairs(mod_map_db) do
    if not registries.state_mod_namespace_keys_lookup[key] then
      log.debug("GC: Removing non-whitelisted mod namespace element: [%s]", key)
      mod_map_db[key] = nil
    end
  end

  for tag in pairs(mod_map_db.timers) do
    if not ctx.relevant_timers_tag_list[tag] then
      log.debug("GC: Removing obsolete timer tag '%s' from mod '%s'", tag, mod_id)
      mod_map_db.timers[tag] = nil
    end
  end

  local mod_critter_tags = registries.critter_active_tags[mod_id] or {}
  for tag in pairs(mod_map_db.objects) do
    if not mod_critter_tags[tag] then
      log.debug("GC: Removing obsolete critter tag '%s' from mod '%s'", tag, mod_id)
      mod_map_db.objects[tag] = nil
    end
  end

  if next(mod_map_db.objects) == nil and next(mod_map_db.timers) == nil then
    log.debug("GC: Removing maps[%d][%s] mod namespace", ck.map_id, mod_id)
    ctx.current_map[mod_id] = nil
  end
end

-- Garbage Collection ✨
-- maps
function gc.purge_maps(db, ctx)
  if next(ctx.current_map) == nil then
    log.debug("GC: Removing maps[%d]", ck.map_id)
    db.maps[ck.map_id] = nil
  end

  local ffi = require('ffi')
  for map_id, _ in pairs(db.maps) do
    if map_id == ck.map_id then goto continue end

    if not ffi.C.ck_config_is_map_savable(map_id) then
      log.debug("GC: Removing 'saved=no' map[%d]", map_id)
      db.maps[map_id] = nil
    end

    ::continue::
  end
end

return gc
