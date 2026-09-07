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

  function logger.header(message, ...)
    local formatted = (select('#', ...) > 0) and string.format(message, ...) or message
    local msg_str = tostring(formatted)

    -- "=" line to match text length
    local border = string.rep("=", string.len(msg_str))
    local colored_border = colorize(ANSI.CYAN, border)

    print(string.format("%s %s\n%s %s\n%s %s",
      formatted_prefix, colored_border,
      formatted_prefix, msg_str,
      formatted_prefix, colored_border))
  end

  function logger.fixed_header(message, ...)
    local formatted = (select('#', ...) > 0) and string.format(message, ...) or message
    local total_width = 40

    local msg_str = tostring(formatted)
    local msg_length = string.len(msg_str)

    local border = string.rep("=", total_width)
    local colored_border = colorize(ANSI.CYAN, border)

    -- centers text in fixed width
    local spaces_needed = total_width - msg_length
    if spaces_needed > 0 then
      local left_spaces = math.floor(spaces_needed / 2)
      local right_spaces = spaces_needed - left_spaces
      msg_str = string.rep(" ", left_spaces) .. msg_str .. string.rep(" ", right_spaces)
    end

    print(string.format("%s %s\n%s %s\n%s %s",
      formatted_prefix, colored_border,
      formatted_prefix, msg_str,
      formatted_prefix, colored_border))
  end

  return logger
end

return log_module
