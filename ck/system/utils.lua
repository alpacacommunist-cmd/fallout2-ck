local utils = {}

function utils.print_table(t, log, indent)
  indent = indent or 0
  for k, v in pairs(t) do
    local formatting = string.rep("  ", indent) .. k .. ": "
    if type(v) == "table" then
      log.debug(formatting)
      utils.print_table(v, log, indent + 1)
    else
      log.debug(formatting .. tostring(v))
    end
  end
end

function utils.shallow_copy(orig)
  if type(orig) ~= 'table' then return orig end

  local copy = {}
  for k, v in pairs(orig) do
    copy[k] = v
  end
  return copy
end

function utils.deep_copy(orig)
  local orig_type = type(orig)
  local copy

  if orig_type == 'table' then
    copy = {}
    for orig_key, orig_value in next, orig, nil do
      copy[utils.deep_copy(orig_key)] = utils.deep_copy(orig_value)
    end
    setmetatable(copy, utils.deep_copy(getmetatable(orig)))
  else
    copy = orig
  end

  return copy
end

function utils.make_readonly(orig_table, log_context)
  if type(orig_table) ~= "table" then return orig_table end

  local proxy = {}
  local mt = {
    __index = function(_, key)
      local value = orig_table[key]
      if type(value) == "table" then
        return utils.make_readonly(value, log_context)
      end
      return value
    end,

    __newindex = function(_, key, value)
      local msg = string.format(
        "[%s] ERROR: Attempt to modify ReadOnly state field '%s' = '%s'!",
        tostring(log_context), tostring(key), tostring(value)
      )

      error(msg .. "\n" .. debug.traceback())
    end,

    __metatable = "Access Denied"
  }

  setmetatable(proxy, mt)
  return proxy
end

---checks if val is nil, empty or only consists of spaces
---@param val any
---@return boolean
function utils.is_blank(val)
  if val == nil then
    return true
  end

  local str = tostring(val)

  return str:match("^%s*$") ~= nil
end

return utils
