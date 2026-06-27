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

return utils
