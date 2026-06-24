local player = require('ck.fallout2.player')
local    T = i18n.bind('arroyo_expanded')

local dialogs = { alice_nodes = {}, minoc_fn = {} }

dialogs.alice_nodes = {
  init = function(ctx)
    ctx.reply(T["alice.init_reply"])

    if player.stats.intelligence > 7 then
      ctx.option(T["alice.init_opt_tech"], "talk_tech", "GOOD")
    else
      ctx.option(T["alice.init_opt_dumb"], "dumb_reply", "NEUTRAL")
    end

    ctx.option(T["alice.init_opt_rob"], "robbery_attempt", "BAD")
    ctx.option(T["alice.init_opt_exit"], "exit_friendly")
  end,

  talk_tech = function(ctx)
    ctx.reply(T["alice.talk_tech_reply"])

    ctx.option(T["alice.talk_tech_opt_nodes"], "talk_nodes", "GOOD")
    ctx.option(T["alice.talk_tech_opt_quest"], "ask_quest", "NEUTRAL")
    ctx.option(T["alice.talk_tech_opt_exit"], "exit_friendly")
  end,

  talk_nodes = function(ctx)
    ctx.reply(T["alice.talk_nodes_reply"])

    ctx.option(T["alice.talk_nodes_opt_back"], "talk_tech", "NEUTRAL")
    ctx.option(T["alice.talk_nodes_opt_exit"], "exit_friendly", "GOOD")
  end,

  dumb_reply = function(ctx)
    ctx.reply(T["alice.dumb_reply_reply"])

    ctx.option(T["alice.dumb_reply_opt_torr"], "dumb_exit", "GOOD")
    ctx.option(T["alice.dumb_reply_opt_exit"], "dumb_exit", "NEUTRAL")
  end,

  robbery_attempt = function(ctx)
    ctx.reply(T["alice.robbery_reply"])

    ctx.option(T["alice.robbery_opt_easy"], "talk_tech", "GOOD")
    ctx.option(T["alice.robbery_opt_kill"], "combat_trigger", "BAD")
  end,

  ask_quest = function(ctx)
    ctx.reply(T["alice.ask_quest_reply"])

    ctx.option(T["alice.ask_quest_opt_yes"], "quest_accepted", "GOOD")
    ctx.option(T["alice.ask_quest_opt_no"], "talk_tech", "NEUTRAL")
  end,

  quest_accepted = function(ctx)
    ctx.reply(T["alice.quest_acc_reply"])
    ctx.option(T["alice.quest_acc_opt_leave"], "exit_friendly")
  end,

  dumb_exit = function(ctx)
    ctx.exit()
  end,

  exit_friendly = function(ctx)
    ctx.reply(T["alice.exit_friendly_reply"])
    ctx.exit()
  end,

  combat_trigger = function(ctx)
    ctx.exit()
  end
}

dialogs.minoc_fn.en = function(ctx)
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
end

return dialogs
