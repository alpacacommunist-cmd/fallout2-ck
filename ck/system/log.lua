local log_module = {}

log_module.debug_enabled = true
log_module.use_colors    = true

local ANSI = {
  RESET  = "\27[0m",
  BOLD   = "\27[1m",
  RED    = "\27[31m",
  GREEN  = "\27[32m",
  YELLOW = "\27[33m",
  BLUE    = "\27[34m",
  MAGENTA = "\27[35m",
  CYAN   = "\27[36m",
  WHITE  = "\27[37m",
  GRAY   = "\27[90m"
}

log_module.Style = {
  Minimal   = "minimal",
  Cyberpunk = "cyberpunk",
  Modern    = "modern"
}

local function colorize(color_code, text)
  if log_module.use_colors then
    return color_code .. text .. ANSI.RESET
  end
  return text
end

local function utf8_len(str)
  if utf8 and utf8.len then
    return utf8.len(str) or string.len(str)
  end
  return string.len(str)
end

function log_module.new(prefix)
  local logger = {}

  local formatted_prefix = colorize(ANSI.BOLD .. ANSI.WHITE, "[" .. prefix .. "]")
  local clean_prefix_len = utf8_len(prefix) + 2

  function logger.header(style, message, ...)
    if type(style) ~= "string" or (style ~= "minimal" and style ~= "cyberpunk" and style ~= "modern") then
      return logger.header(log_module.Style.Minimal, style, message, ...)
    end

    local formatted_title = (select('#', ...) > 0) and string.format(message, ...) or message
    local total_width = 80

    if style == log_module.Style.Minimal then
      local used_len = 4 + clean_prefix_len + 5 + utf8_len(formatted_title) + 1
      local fill_chars = (total_width > used_len) and (total_width - used_len) or 0

      local line_decor = colorize(ANSI.BLUE, "───")
      local mid_decor  = colorize(ANSI.BLUE, " ─── ")
      local fill_line  = colorize(ANSI.BLUE, string.rep("─", fill_chars))
      local title_part = colorize(ANSI.BOLD .. ANSI.WHITE, formatted_title)

      print(string.format("%s %s%s %s %s", line_decor, formatted_prefix, mid_decor, title_part, fill_line))

    elseif style == log_module.Style.Cyberpunk then
      local used_len = 4 + clean_prefix_len + 5 + utf8_len(formatted_title) + 1
      local fill_chars = (total_width > used_len) and (total_width - used_len) or 0

      local block_decor = colorize(ANSI.BRIGHT_GREEN, "▒▒▒")
      local mid_decor   = colorize(ANSI.BRIGHT_GREEN, " ▒▒▒ ")
      local fill_line   = colorize(ANSI.BRIGHT_GREEN, string.rep("▒", fill_chars))
      local title_part  = colorize(ANSI.BOLD .. ANSI.BRIGHT_GREEN, formatted_title)

      print(string.format("%s %s%s %s %s", block_decor, formatted_prefix, mid_decor, title_part, fill_line))

    elseif style == log_module.Style.Modern then
      local arrow1 = colorize(ANSI.YELLOW, "❯❯❯")
      local arrow2 = colorize(ANSI.YELLOW, "❯")
      local title_part = colorize(ANSI.BOLD, formatted_title)

      print(string.format("%s %s %s %s", arrow1, formatted_prefix, arrow2, title_part))
    end
  end

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
