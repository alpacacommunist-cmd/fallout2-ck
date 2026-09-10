-- ck/fallout2/dialogue.lua
local ffi = require("ffi")
local log = ck.log.new('CK Dialogue')

local dialogue = {
  -- npc_id -> dialog function
  registry = {},

  reactions = { GOOD = 49, NEUTRAL = 50, BAD = 51 }
}

-- TODO: add mod_id
function dialogue.register(lua_id, fn_or_nodes)
  dialogue.registry[lua_id] = fn_or_nodes
  log.info("Registered dialogue for npc: " .. tostring(lua_id))
end

function dialogue.add_option(text, reaction)
  ffi.C.ck_dialog_add_option(text, reaction or dialogue.reactions.NEUTRAL)
end

function dialogue.exit()
  ffi.C.ck_dialog_exit()
  ffi.C.ck_dialog_close_ui()
end

function dialogue.clear_dialogs()
  dialogue.registry = {}
  log.info("Cleared dialogues registry")
end

function dialogue.is_registered(npc_id) return dialogue.registry[npc_id] ~= nil end

function dialogue.say(text)
  ffi.C.ck_dialog_set_reply(text)
  ffi.C.ck_dialog_add_option("[Continue]")
  ffi.C.ck_dialog_go()
end

function dialogue.ask(text, options)
  ffi.C.ck_dialog_set_reply(text)

  for _, opt in ipairs(options) do ffi.C.ck_dialog_add_option(opt) end

  local choice = dialogue.go()
  return choice + 1  -- 1-based
end

--
-- Nodes engine
--

local function run_node_dialogue(npc_id, nodes)
  local current_node = "init"
  local active = true

  local ctx = { npc_id = npc_id }

  local current_options = {}

  function ctx.reply(text)
    ffi.C.ck_dialog_set_reply(text)
    current_options = {}
  end

  function ctx.option(text, next_node_name, reaction_or_nil)
    table.insert(current_options, next_node_name)

    local r_type = reaction_or_nil or "NEUTRAL"
    local c_reaction = dialogue.reactions[r_type] or dialogue.reactions.NEUTRAL

    ffi.C.ck_dialog_add_option(text, c_reaction)
  end

  function ctx.exit()
    active = false
  end

  while active do
    local node_fn = nodes[current_node]

    if not node_fn then
      log.error(string.format("Node '%s' not found for npc %s", tostring(current_node), tostring(npc_id)))
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

function dialogue.start(npc_id)
  local target = dialogue.registry[npc_id]

  if not target then
    log.error("No dialogue registered for npc: " .. tostring(npc_id))
    return
  end

  if not ffi.C.ck_dialog_init_ui() then
    log.error("Failed to init dialogue UI for npc: " .. tostring(npc_id))
    return
  end

  if type(target) == "table" then
    run_node_dialogue(npc_id, target)
  elseif type(target) == "function" then
    target({ npc_id = npc_id })
  end

  dialogue.exit()
end

return dialogue
