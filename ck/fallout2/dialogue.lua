-- ck/fallout2/dialogue.lua
local dialogue = {}

-- npcId -> dialog function
local registry = {}

function dialogue.register(npc_id, fn)
  registry[npc_id] = fn
  print("[CK Dialogue] Registered dialogue for npc: " .. tostring(npc_id))
end

function dialogue.say(text)
  ck.dialog.set_reply(text)
  ck.dialog.add_option("[Continue]")
  ck.dialog.go()
end

function dialogue.ask(text, options)
  ck.dialog.set_reply(text)
  for _, opt in ipairs(options) do
    ck.dialog.add_option(opt)
  end
  local choice = ck.dialog.go()
  return choice + 1  -- 1-based
end

function dialogue.start(npc_id)
  local fn = registry[npc_id]
  if not fn then
    print("[CK Dialogue] No dialogue registered for npc: " .. tostring(npc_id))
    return
  end

  fn({ npcId = npc_id })

  ck.dialog.exit()
end

return dialogue
