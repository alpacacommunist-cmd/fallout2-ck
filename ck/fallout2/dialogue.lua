-- ck/fallout2/dialogue.lua
local ffi = require("ffi")
ffi.cdef[[
  bool ck_dialog_init_ui();
  void ck_dialog_set_reply(const char* text);
  void ck_dialog_add_option(const char* text, int reaction);
  int  ck_dialog_go();
  void ck_dialog_exit();
  void ck_dialog_close_ui();
]]

local C = ffi.C

ck.dialogue.set_reply    = C.ck_dialog_set_reply
ck.dialogue.add_option   = C.ck_dialog_add_option
ck.dialogue.go           = C.ck_dialog_go
ck.dialogue.exit         = C.ck_dialog_exit
ck.dialogue.init_ui      = C.ck_dialog_init_ui
ck.dialogue.close_ui     = C.ck_dialog_close_ui

local dialogue = {
  reactions = { GOOD = 49, NEUTRAL = 50, BAD = 51 }
}

dialogue.set_reply = ck.dialogue.set_reply
dialogue.go        = ck.dialogue.go
dialogue.init_ui   = ck.dialogue.init_ui
dialogue.close_ui  = ck.dialogue.close_ui

function dialogue.add_option(text, reaction)
  ck.dialogue.add_option(text, reaction or dialogue.reactions.NEUTRAL)
end

function dialogue.exit()
  ck.dialogue.exit()
  dialogue.close_ui()
end

local log = ck.log.new('CK Dialogue')

-- npc_id -> dialog function
local registry = {}

function dialogue.register(npc_id, fn_or_nodes)
  registry[npc_id] = fn_or_nodes
  log.info("Registered dialogue for npc: " .. tostring(npc_id))
end

function dialogue.is_registered(npc_id) return registry[npc_id] ~= nil end

function dialogue.say(text)
  dialogue.set_reply(text)
  dialogue.add_option("[Continue]")
  dialogue.go()
end

function dialogue.ask(text, options)
  dialogue.set_reply(text)

  for _, opt in ipairs(options) do dialogue.add_option(opt) end

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
    dialogue.set_reply(text)
    current_options = {}
  end

  function ctx.option(text, next_node_name, reaction_or_nil)
    table.insert(current_options, next_node_name)

    local r_type = reaction_or_nil or "NEUTRAL"
    local c_reaction = dialogue.reactions[r_type] or dialogue.reactions.NEUTRAL

    dialogue.add_option(text, c_reaction)
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

    local chosen_c_index = dialogue.go()

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
  local target = registry[npc_id]

  if not target then
    log.error("No dialogue registered for npc: " .. tostring(npc_id))
    return
  end

  if not dialogue.init_ui() then
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
