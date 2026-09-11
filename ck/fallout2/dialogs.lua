-- ck/fallout2/dialog.lua
local ffi = require("ffi")
local log = ck.log.new('CK Dialogue')

local registries = require("ck.system.registries")

local dialog = {
  -- lua_id -> dialog function
  registry = registries.dialogs,

  reactions = { good = 49, neutral = 50, bad = 51 }
}

-- TODO: add mod_id
function dialog.register(lua_id, fn_or_nodes)
  dialog.registry[lua_id] = fn_or_nodes
  log.info("Registered dialog for npc: " .. tostring(lua_id))
end

function dialog.add_option(text, reaction)
  ffi.C.ck_dialog_add_option(text, reaction or dialog.reactions.neutral)
end

function dialog.exit()
  ffi.C.ck_dialog_exit()
  ffi.C.ck_dialog_close_ui()
end

function dialog.is_registered(lua_id) return dialog.registry[lua_id] ~= nil end

function dialog.say(text)
  ffi.C.ck_dialog_set_reply(text)
  ffi.C.ck_dialog_add_option("[Continue]")
  ffi.C.ck_dialog_go()
end

function dialog.ask(text, options)
  ffi.C.ck_dialog_set_reply(text)

  for _, opt in ipairs(options) do ffi.C.ck_dialog_add_option(opt) end

  local choice = dialog.go()
  return choice + 1  -- 1-based
end

-- Context class
local Context = {}
Context.__index = Context

Context.new = function(lua_id)
  local self = setmetatable({}, Context)
  local player = require('ck.fallout2.player')

  self.lua_id = lua_id
  self.active = true

  self.current_options = {}

  self.player = player

  return self
end

function Context:reply(text)
  ffi.C.ck_dialog_set_reply(text)

  for option = 1, #self.current_options do self.current_options[option] = nil end
end

function Context:option(text, next_node_name, reaction)
  table.insert(self.current_options, next_node_name)

  local reaction_type = reaction or 'neutral'
  local reaction = dialog.reactions[reaction_type]

  dialog.add_option(text, reaction)
end

function Context:exit()
  self.active = false
end

-- Nodes engine
local function run_node_dialog(lua_id, nodes)
  local current_node = "init"
  local context = Context.new(lua_id)

  while context.active do
    local node_fn = nodes[current_node]

    if not node_fn then
      log.error(string.format("Node '%s' not found for npc %s", tostring(current_node), tostring(lua_id)))
      break
    end

    node_fn(context)

    if not context.active then break end
    if #context.current_options == 0 then break end

    local chosen_index = ffi.C.ck_dialog_go()

    local chosen_lua_index = chosen_index + 1
    local next_node = context.current_options[chosen_lua_index]

    if next_node then
      current_node = next_node
    else
      log.error("Unknown option index: " .. tostring(chosen_c_index) .. ". Exiting.")
      context.active = false
    end
  end
end

function dialog.start(lua_id)
  local target = dialog.registry[lua_id]

  if not target then
    log.error("No dialog registered for npc: " .. tostring(lua_id))
    return
  end

  if not ffi.C.ck_dialog_init_ui() then
    log.error("Failed to init dialog UI for npc: " .. tostring(lua_id))
    return
  end

  if type(target) == "table" then
    run_node_dialog(lua_id, target)
  elseif type(target) == "function" then
    target({ lua_id = lua_id })
  end

  dialog.exit()
end

return dialog
