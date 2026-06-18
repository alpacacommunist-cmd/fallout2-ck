-- mods/arroyo_expanded/init.lua
print("[Mod] Loading Arroyo Expanded Mod...")

local events      = require('ck.fallout2.events')
local gameTime    = require('ck.fallout2.game_time')
local log         = require('ck.fallout2.log')
local map         = require('ck.fallout2.map')
local i18n        = require('ck.fallout2.i18n')
local dialogue    = require('ck.fallout2.dialogue')

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
    description = "You see Alice. She knows erlang"
  })

  print("[CK] NPC ID: " .. tostring(npc_id))

  dialogue.register(npc_id, {
    init = function(ctx)
      ctx.reply("You see a young woman typing furiously on a pip-boy-like device. She looks up. 'Hello, traveler. I'm Alice. I'm currently refactoring a distributed wasteland network. Do you happen to know Erlang?'")

      if true then
        ctx.option("Erlang? Fascinating. Are you utilizing the Actor model for concurrent message passing?", "talk_tech", "GOOD")
      else
        ctx.option("Er... lang? Is that some kind of pre-war energy drink?", "dumb_reply", "NEUTRAL")
      end

      ctx.option("I don't care about your gibberish. Hand over your caps!", "robbery_attempt", "BAD")
      ctx.option("Just passing through. Goodbye.", "exit_friendly")
    end,

    talk_tech = function(ctx)
      ctx.reply("'Exactly! isolated processes, no shared state, pure asynchronous bliss. It's the only way to keep the wasteland's communications fault-tolerant. But the local shamans keep saying it's black magic.'")

      ctx.option("Tell me more about how you handle node failures.", "talk_nodes", "GOOD")
      ctx.option("Sounds like a lot of work. Can I help you with anything?", "ask_quest", "NEUTRAL")
      ctx.option("You are completely crazy. I'm leaving.", "exit_friendly")
    end,

    talk_nodes = function(ctx)
      ctx.reply("'Let it crash! That's our motto. Supervisors will restart the failed processes automatically. It's much better than defensive programming.' She smiles warmly.")

      ctx.option("Brilliant approach. Let's talk about something else.", "talk_tech", "NEUTRAL")
      ctx.option("Alright, I must go. Keep up the good work!", "exit_friendly", "GOOD")
    end,

    dumb_reply = function(ctx)
      ctx.reply("Alice sighs deeply, looking disappointed. 'No, it's not a drink. It's a programming language. Go talk to Torr, I think you two would find a lot of common ground.'")

      ctx.option("Me like Torr! Torr guard moomoo!", "dumb_exit", "GOOD")
      ctx.option("Ug... drink sound better. Bye.", "dumb_exit", "NEUTRAL")
    end,

    robbery_attempt = function(ctx)
      ctx.reply("Alice steps back, her hand dropping near a concealed laser pistol. 'Try it, tribal. And my defense processes will terminate your lifecycle before you can even blink.'")

      ctx.option("Whoa, easy! I was just joking.", "talk_tech", "GOOD")
      ctx.option("You and what army? Prepare to die!", "combat_trigger", "BAD")
    end,

    ask_quest = function(ctx)
      ctx.reply("'Actually, yes. A pack of geckos chewed through my external antenna array. If you clear them out, I can reward you with some pre-war electronic scrap. Deal?'")

      ctx.option("Consider it done. I'll be back soon.", "quest_accepted", "GOOD")
      ctx.option("No thanks, find another errand boy.", "talk_tech", "NEUTRAL")
    end,

    quest_accepted = function(ctx)
      ctx.reply("'Excellent. Don't let them bite you. Radiation is bad for your state.'")
      ctx.option("[Leave]", "exit_friendly")
    end,

    dumb_exit = function(ctx)
      ctx.exit()
    end,

    exit_friendly = function(ctx)
      ctx.reply("'Safe travels. Remember: let it crash!'")
      ctx.exit()
    end,

    combat_trigger = function(ctx)
      ctx.exit()
      log.print("COMBAT STARTED WITH NPC: " .. tostring(ctx.npc_id))
    end

  })

  -- dialogue.register(npc_id, function(ctx)
  --   local askedAboutQuest = false
  --
  --   -- inner main manu function for returns
  --   local function mainMenu()
  --     local options = {
  --       "Who are you?",
  --       "Tell me about this place.",
  --     }
  --
  --     -- quest option only if not asked before
  --     if not askedAboutQuest then
  --       table.insert(options, "Do you need any help?")
  --     end
  --
  --     table.insert(options, "Goodbye.")
  --
  --     local choice = dialogue.ask("Hello, traveler. What do you want?", options)
  --
  --     if choice == 1 then
  --       -- branch 1
  --       local c = dialogue.ask(
  --         "I am Mynoc, keeper of the bridge. " ..
  --         "I have watched over this village for many years.",
  --         {
  --           "How long have you been here?",
  --           "Sounds lonely.",
  --           "I see. Goodbye."
  --         }
  --       )
  --
  --       if c == 1 then
  --         dialogue.say("Since before you were born, young one. " ..
  --         "The wastes have a way of aging a man.")
  --       elseif c == 2 then
  --         dialogue.say("Lonely? No. The river keeps me company. " ..
  --         "And the occasional traveler, like yourself.")
  --       end
  --
  --       -- back to main menu
  --       mainMenu()
  --
  --     elseif choice == 2 then
  --       dialogue.say(
  --         "This is Arroyo, village of the Chosen One. " ..
  --         "We are a peaceful people. The wastes beyond are not so kind."
  --       )
  --       mainMenu()
  --
  --     elseif choice == 3 and not askedAboutQuest then
  --       -- quest branch
  --       askedAboutQuest = true
  --
  --       local c = dialogue.ask(
  --         "Actually... yes. A brahmin wandered off two days ago. " ..
  --         "Old Sulik is worried sick. Would you find it?",
  --         {
  --           "Sure, I'll look for it.",
  --           "Sorry, I'm busy."
  --         }
  --       )
  --
  --       if c == 1 then
  --         dialogue.say("Thank you, traveler. Last seen near the hunting grounds.")
  --         -- ctx.quest.start('find_brahmin')  -- for future quest api
  --       else
  --         dialogue.say("I understand. Safe travels.")
  --       end
  --     else
  --       -- bye
  --       dialogue.say("Safe travels, friend.")
  --       return
  --     end
  --   end
  --
  --   mainMenu()
  -- end)

end)
