local ffi = require("ffi")

local encoding = {}

function encoding.cp1251_to_utf8(raw_c_str)
  if raw_c_str == nil or raw_c_str == ffi.NULL then
    return ""
  end

  local in_len = ffi.C.strlen(raw_c_str)
  if in_len == 0 then return "" end

  local max_size = (in_len * 2) + 1
  local buf = ffi.new("char[?]", max_size)
  local written = ffi.C.ck_cp1251_to_utf8(raw_c_str, buf, max_size)

  return ffi.string(buf, written)
end

return encoding
