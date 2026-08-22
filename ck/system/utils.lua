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

return utils
