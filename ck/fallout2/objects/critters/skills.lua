local ffi = require('ffi')

ffi.cdef[[
    void ck_get_skills_metadata(void (*callback)(const char* name, int value));
]]

local SKILLS_MAP = {}

local callback = ffi.cast("void (*)(const char*, int)", function(lua_name_ptr, value)
    SKILLS_MAP[ffi.string(lua_name_ptr)] = value
end)

ffi.C.ck_get_skills_metadata(callback)
callback:free()

local skills = { MAP = SKILLS_MAP }

function skills.create_proxy(read_skill_fn)
  local proxy = {}
  setmetatable(proxy, {
    __index = function(_, key)
      local c_skill = SKILLS_MAP[key]
      if    c_skill then return read_skill_fn(c_skill) end

      return nil
    end
  })

  return proxy
end

return skills
