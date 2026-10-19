local ffi = require('ffi')
local log = require('ck/system/log').new('stats.lua')

local SKILLS_MAP    = {} -- name -> int
local SKILLS_ID_MAP = {} -- int -> name

local callback = ffi.cast("void (*)(const char*, int)", function(lua_name_ptr, value)
  local name = ffi.string(lua_name_ptr)

  SKILLS_MAP[name]     = value
  SKILLS_ID_MAP[value] = name
end)

ffi.C.ck_get_skills_metadata(callback)
callback:free()

local skills = { MAP = SKILLS_MAP, ID_MAP = SKILLS_ID_MAP }

function skills.create_proxy(read_total_fn, write_total_fn, read_base_fn)
  local proxy = {}

  local actual_read_base = read_base_fn or read_total_fn
  local actual_write_base = write_total_fn

  setmetatable(proxy, {
    __index = function(_, key)
      local is_base_request = key:match("(.+)_base$")

      if is_base_request then
        local c_skill_id = skills.MAP[is_base_request]

        if c_skill_id and actual_read_base then
          return actual_read_base(c_skill_id)
        end
      else
        local c_skill_id = skills.MAP[key]

        if c_skill_id and read_total_fn then
          return read_total_fn(c_skill_id)
        end
      end

      return nil
    end,

    __newindex = function(_, key, value)
      local clean_key = key:match("(.+)_base$") or key
      local c_skill_id = skills.MAP[clean_key]

      if c_skill_id then
        if type(value) ~= "number" then
          log.error("Skill '%s' must be a number", clean_key)
          return
        end

        if actual_write_base then
          actual_write_base(c_skill_id, value)
        else
          log.error("Skill '%s' is read-only for this object", clean_key)
        end
      else
        log.error("Unknown skill name: '%s'", tostring(key))
      end
    end
  })

  return proxy
end

return skills
