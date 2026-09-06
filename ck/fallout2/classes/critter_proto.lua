local ProtoClass = {}
ProtoClass.__index = ProtoClass

function ProtoClass.new(pid, name, description, ai_packet)
  local self = setmetatable({}, ProtoClass)
  self.pid   = pid
  self.name  = name
  self.description = description
  self.ai_packet = ai_packet

  self.c_ptr = ffi.C.ck_critter_get_proto_by_pid(self.pid)

  self.stats = stats.create_proxy(
    function(stat_id)        return ffi.C.ck_critter_proto_get_base_stat(self.c_ptr, stat_id) end,
    function(stat_id, value) ffi.C.ck_critter_proto_set_base_stat(self.c_ptr, stat_id, value) end
  )

  self.skills = skills.create_proxy(
    function(skill_id)        return ffi.C.ck_critter_proto_get_skill(self.c_ptr, skill_id) end,
    function(skill_id, value) ffi.C.ck_critter_proto_set_skill(self.c_ptr, skill_id, value) end
  )

  return self
end

function ProtoClass:set_stats(stats_table)
  if type(stats_table) ~= "table" then
    error("set_stats expects table, got: " .. type(stats_table))
  end

  for key, value in pairs(stats_table) do
    self.stats[key] = value
  end

  return self
end

function ProtoClass:set_skills(skills_table)
  if type(skills_table) ~= "table" then
    error("set_skills expects table, got: " .. type(skills_table))
  end

  for key, value in pairs(skills_table) do
    self.skills[key] = value
  end

  return self
end

return ProtoClass

