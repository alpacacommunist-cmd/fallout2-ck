local ffi = require('ffi')

ffi.cdef[[
    void ck_get_skills_metadata(void (*callback)(const char* name, int value));
]]

local SKILLS_MAP = {}

local callback = ffi.cast("void (*)(const char*, int)", function(name, value)
    SKILLS_MAP[ffi.string(name)] = value
end)

local skills = { MAP = SKILLS_MAP }

ffi.C.ck_get_skills_metadata(callback)
callback:free()

return skills
