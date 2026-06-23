local log_module = {}

log_module.debug_enabled = true
log_module.use_colors    = true

local ANSI = {
  RESET  = "\27[0m",
  BOLD   = "\27[1m",
  RED    = "\27[31m",
  GREEN  = "\27[32m",
  YELLOW = "\27[33m",
  CYAN   = "\27[36m",
  WHITE  = "\27[37m",
  GRAY   = "\27[90m"
}

local function colorize(color_code, text)
  if log_module.use_colors then
    return color_code .. text .. ANSI.RESET
  end
  return text
end

function log_module.new(prefix)
  local logger = {}

  local formatted_prefix = colorize(ANSI.BOLD .. ANSI.WHITE, "[" .. prefix .. "]")

  function logger.info(message, ...)
    local formatted = (select('#', ...) > 0) and string.format(message, ...) or message
    local tag = colorize(ANSI.GREEN, "[INFO]")
    print(string.format("%s %s %s", formatted_prefix, tag, tostring(formatted)))
  end

  function logger.debug(message, ...)
    if not log_module.debug_enabled then return end

    local formatted = (select('#', ...) > 0) and string.format(message, ...) or message
    local tag = colorize(ANSI.GRAY, "[DEBUG]")
    local text = colorize(ANSI.GRAY, tostring(formatted))
    print(string.format("%s %s %s", formatted_prefix, tag, text))
  end

  function logger.warn(message, ...)
    local formatted = (select('#', ...) > 0) and string.format(message, ...) or message
    local tag = colorize(ANSI.YELLOW, "[WARN]")
    print(string.format("%s %s %s", formatted_prefix, tag, tostring(formatted)))
  end

  function logger.error(message, ...)
    local formatted = (select('#', ...) > 0) and string.format(message, ...) or message
    local tag = colorize(ANSI.BOLD .. ANSI.RED, "[ERROR]")
    local text = colorize(ANSI.RED, tostring(formatted))
    print(string.format("%s %s %s", formatted_prefix, tag, text))
  end

  return logger
end

return log_module
