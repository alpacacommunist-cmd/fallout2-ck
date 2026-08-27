local ffi = require('ffi')

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

function skills.create_proxy(read_skill_fn, write_skill_fn)
  local proxy = {}

  local mt = {}

  if read_skill_fn then
    mt.__index = function(_, key)
      local c_skill = SKILLS_MAP[key]
      if c_skill then return read_skill_fn(c_skill) end
      return nil
    end
  end

  if write_skill_fn then
    mt.__newindex = function(_, key, value)
      local c_skill = SKILLS_MAP[key]
      if c_skill then
        if type(value) ~= "number" then
          error(string.format("Skill '%s' must be a number, got %s", key, type(value)))
        end
        write_skill_fn(c_skill, value)
      else
        error(string.format("Unknown skill name: '%s'", tostring(key)))
      end
    end
  end

  setmetatable(proxy, mt)

  return proxy
end

return skills
