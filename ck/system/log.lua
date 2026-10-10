local ffi = require("ffi")

local Level = { Info = 0, Success = 1, Debug = 2, Warning = 3, Error = 4 }
local Style = { Normal = 0, Rule = 1, Minimal = 2, Banner = 3, Raw = 4 }
local Source = { Native = 0, Lua = 1 }

local Logger = {}
Logger.__index = Logger

function Logger.new(level, context, style)
  local self = setmetatable({}, Logger)
  self.level = level
  self.context = context
  self.style = style or Style.Normal
  return self
end

function Logger:__call(fmt, ...)
  local message

  local args_count = select('#', ...)

  if args_count > 0 then
    message = string.format(tostring(fmt), ...)
  else
    message = tostring(fmt or "")
  end

  ffi.C.ck_system_log(self.level, Source.Lua, self.style, self.context, message)
end

function Logger:rule(fmt, ...)
    local spawned = Logger.new(self.level, self.context, Style.Rule)
    if fmt then spawned(fmt, ...) end
    return spawned
end

function Logger:minimal(fmt, ...)
    local spawned = Logger.new(self.level, self.context, Style.Minimal)
    if fmt then spawned(fmt, ...) end
    return spawned
end

function Logger:banner(fmt, ...)
    local spawned = Logger.new(self.level, self.context, Style.Banner)
    if fmt then spawned(fmt, ...) end
    return spawned
end

local Context = {}
Context.__index = Context

function Context.new(name)
  local self = setmetatable({}, Context)

  self.info    = Logger.new(Level.Info, name)
  self.success = Logger.new(Level.Success, name)
  self.debug   = Logger.new(Level.Debug, name)
  self.warning = Logger.new(Level.Warning, name)
  self.error   = Logger.new(Level.Error, name)
  return self
end


return {
  new = function(name)
    return Context.new(name)
  end,

  info    = Logger.new(Level.Info, ""),
  success = Logger.new(Level.Success, ""),
  debug   = Logger.new(Level.Debug, ""),
  warning = Logger.new(Level.Warning, ""),
  error   = Logger.new(Level.Error, ""),
}
