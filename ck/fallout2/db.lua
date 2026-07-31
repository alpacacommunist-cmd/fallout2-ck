local ffi = require("ffi")

local db = {}

local function parse_ffi_struct(info_ffi)
  return {
    pid         = info_ffi.pid,
    fid         = info_ffi.fid,
    type        = info_ffi.type,
    sid         = info_ffi.sid,

    name        = ffi.string(info_ffi.name),
    filename    = ffi.string(info_ffi.filename),
    description = ffi.string(info_ffi.description)
  }
end

function db.get_by_pid(pid)
  local info_ffi = ffi.new("CkProtoInfoFFI")
  if ffi.C.ck_proto_get_by_pid(pid, info_ffi) then
    return parse_ffi_struct(info_ffi)
  end

  return nil
end

function db.get_by_name(name, type)
  type = type or -1
  local info_ffi = ffi.new("CkProtoInfoFFI")
  if ffi.C.ck_proto_get_by_name(name, type, info_ffi) then
    return parse_ffi_struct(info_ffi)
  end

  return nil
end

function db.get_by_type(obj_type, max_results)
  max_results = max_results or 32
  
  local array_ffi = ffi.new("CkProtoInfoFFI[?]", max_results)
  local count     = ffi.C.ck_proto_get_by_type(obj_type, array_ffi, max_results)
  
  local results = {}
  for i = 0, count - 1 do
    local info = array_ffi[i]

    table.insert(results, {
      pid         = info.pid,
      fid         = info.fid,
      type        = info.type,
      sid         = info.sid,
      name        = ffi.string(info.name),
      filename    = ffi.string(info.filename),
      description = ffi.string(info.description)
    })
  end
  
  return results
end

return db
