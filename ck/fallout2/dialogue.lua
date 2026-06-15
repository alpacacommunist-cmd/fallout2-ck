-- ck/fallout2/dialogue.lua
local ffi = require("ffi")
ffi.cdef[[
    void ck_dialog_set_reply(const char* text);
    void ck_dialog_add_option(const char* text, int reaction);
    int  ck_dialog_go();
    void ck_dialog_exit();
]]

local C = ffi.C

ck.dialogue.set_reply    = C.ck_dialog_set_reply
ck.dialogue.add_option   = C.ck_dialog_add_option
ck.dialogue.go           = C.ck_dialog_go
ck.dialogue.exit         = C.ck_dialog_exit

local dialogue = {
  reactions = { GOOD = 49, NEUTRAL = 50, BAD = 51 }
}

function dialogue.add_option(text, reaction)
  ck.dialogue.add_option(text, reaction or dialogue.reactions.NEUTRAL)
end

dialogue.set_reply = ck.dialogue.set_reply
dialogue.go        = ck.dialogue.go
dialogue.exit      = ck.dialogue.exit

-- npcId -> dialog function
local registry = {}

function dialogue.register(npc_id, fn)
  registry[npc_id] = fn
  print("[CK Dialogue] Registered dialogue for npc: " .. tostring(npc_id))
end

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

function dialogue.start(npc_id)
  local fn = registry[npc_id]

  if not fn then
    print("[CK Dialogue] No dialogue registered for npc: " .. tostring(npc_id))
    return
  end

  fn({ npcId = npc_id })

  dialogue.exit()
end

return dialogue
