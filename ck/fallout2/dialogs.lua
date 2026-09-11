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

--
-- Nodes engine
--

local context = {
  new = function(lua_id)
    self.lua_id = lua_id
    self.active = true

    self.current_options = {}
  end,

  reply = function(ctx, text)
    ffi.C.ck_dialog_set_reply(text)

    for option in pairs(ctx.current_options) do ctx.current_options[option] = nil end
  end,

  option = function(ctx, text, next_node_name, reaction)
    table.insert(ctx.current_options, next_node_name)

    local reaction_type = reaction or 'neutral'
    local reaction = dialog.reactions[reaction_type]

    dialog.add_option(text, reaction)
  end,

  exit = function(ctx)
    ctx.active = false
  end
}

local function run_node_dialog(lua_id, nodes)
  local current_node = "init"
  local active = true

  local ctx = { lua_id = lua_id }

  local current_options = {}

  function ctx.reply(text)
    ffi.C.ck_dialog_set_reply(text)
    current_options = {}
  end

  function ctx.option(text, next_node_name, reaction_or_nil)
    table.insert(current_options, next_node_name)

    local r_type = reaction_or_nil or "neutral"
    local c_reaction = dialog.reactions[r_type] or dialog.reactions.neutral

    ffi.C.ck_dialog_add_option(text, c_reaction)
  end

  function ctx.exit()
    active = false
  end

  while active do
    local node_fn = nodes[current_node]

    if not node_fn then
      log.error(string.format("Node '%s' not found for npc %s", tostring(current_node), tostring(lua_id)))
      break
    end

    node_fn(ctx)

    if not active then break end
    if #current_options == 0 then break end

    local chosen_c_index = ffi.C.ck_dialog_go()

    local chosen_lua_index = chosen_c_index + 1
    local next_node = current_options[chosen_lua_index]

    if next_node then
      current_node = next_node
    else
      log.error("Unknown option index: " .. tostring(chosen_c_index) .. ". Exiting.")
      active = false
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
