-- mods/arroyo_expanded/init.lua
print("[Mod] Loading Arroyo Expanded Mod...")

local events      = require('ck.fallout2.events')
local gameTime    = require('ck.fallout2.game_time')
local log         = require('ck.fallout2.log')
local map         = require('ck.fallout2.map')
local i18n        = require('ck.fallout2.i18n')

local dialogue = require('ck.fallout2.dialogue')

events.on('onGameLoaded', function()
end)

events.on('onMapEnter', function()
  local map_id = map.get_id()

  log.print("Map id: " .. tostring(map_id))
  log.print("Entered map!")

  if map_id ~= 4 then return end

  local npc_id = map.register_critter(16777218, 19908, {
    name = "Alice",
    description = "You see Alice."
  })

  local npc_meta = "";

  print("[CK] NPC ID: " .. tostring(npc_id))

  dialogue.register(npc_id, function(ctx)
    local askedAboutQuest = false

    -- inner main manu function for returns
    local function mainMenu()
      local options = {
        "Who are you?",
        "Tell me about this place.",
      }

      -- quest option only if not asked before
      if not askedAboutQuest then
        table.insert(options, "Do you need any help?")
      end

      table.insert(options, "Goodbye.")

      local choice = dialogue.ask("Hello, traveler. What do you want?", options)

      if choice == 1 then
        -- branch 1
        local c = dialogue.ask(
          "I am Mynoc, keeper of the bridge. " ..
          "I have watched over this village for many years.",
          {
            "How long have you been here?",
            "Sounds lonely.",
            "I see. Goodbye."
          }
        )

        if c == 1 then
          dialogue.say("Since before you were born, young one. " ..
          "The wastes have a way of aging a man.")
        elseif c == 2 then
          dialogue.say("Lonely? No. The river keeps me company. " ..
          "And the occasional traveler, like yourself.")
        end

        -- back to main menu
        mainMenu()

      elseif choice == 2 then
        dialogue.say(
          "This is Arroyo, village of the Chosen One. " ..
          "We are a peaceful people. The wastes beyond are not so kind."
        )
        mainMenu()

      elseif choice == 3 and not askedAboutQuest then
        -- quest branch
        askedAboutQuest = true

        local c = dialogue.ask(
          "Actually... yes. A brahmin wandered off two days ago. " ..
          "Old Sulik is worried sick. Would you find it?",
          {
            "Sure, I'll look for it.",
            "Sorry, I'm busy."
          }
        )

        if c == 1 then
          dialogue.say("Thank you, traveler. Last seen near the hunting grounds.")
          -- ctx.quest.start('find_brahmin')  -- for future quest api
        else
          dialogue.say("I understand. Safe travels.")
        end
      else
        -- bye
        dialogue.say("Safe travels, friend.")
        return
      end
    end

    mainMenu()
  end)

end)
