return {
  alice_name = "You see Alice",
  alice_description = "You see some geek-looking female. She knows erlang",

  alice = {
    init_reply = "You see a young woman typing furiously on a pip-boy-like device. She looks up. 'Hello, traveler. I'm Alice. I'm currently refactoring a distributed wasteland network. Do you happen to know Erlang?'",
    init_opt_tech = "Erlang? Fascinating. Are you utilizing the Actor model for concurrent message passing?",
    init_opt_dumb = "Er... lang? Is that some kind of pre-war energy drink?",
    init_opt_rob = "I don't care about your gibberish. Hand over your caps!",
    init_opt_exit = "Just passing through. Goodbye.",

    talk_tech_reply = "'Exactly! isolated processes, no shared state, pure asynchronous bliss. It's the only way to keep the wasteland's communications fault-tolerant. But the local shamans keep saying it's black magic.'",
    talk_tech_opt_nodes = "Tell me more about how you handle node failures.",
    talk_tech_opt_quest = "Sounds like a lot of work. Can I help you with anything?",
    talk_tech_opt_exit = "You are completely crazy. I'm leaving.",

    talk_nodes_reply = "'Let it crash! That's our motto. Supervisors will restart the failed processes automatically. It's much better than defensive programming.' She smiles warmly.",
    talk_nodes_opt_back = "Brilliant approach. Let's talk about something else.",
    talk_nodes_opt_exit = "Alright, I must go. Keep up the good work!",

    dumb_reply_reply = "Alice sighs deeply, looking disappointed. 'No, it's not a drink. It's a programming language. Go talk to Torr, I think you two would find a lot of common ground.'",
    dumb_reply_opt_torr = "Me like Torr! Torr guard moomoo!",
    dumb_reply_opt_exit = "Ug... drink sound better. Bye.",

    robbery_reply = "Alice steps back, her hand dropping near a concealed laser pistol. 'Try it, tribal. And my defense processes will terminate your lifecycle before you can even blink.'",
    robbery_opt_easy = "Whoa, easy! I was just joking.",
    robbery_opt_kill = "You and what army? Prepare to die!",

    ask_quest_reply = "'Actually, yes. A pack of geckos chewed through my external antenna array. If you clear them out, I can reward you with some pre-war electronic scrap. Deal?'",
    ask_quest_opt_yes = "Consider it done. I'll be back soon.",
    ask_quest_opt_no = "No thanks, find another errand boy.",

    quest_acc_reply = "'Excellent. Don't let them bite you. Radiation is bad for your state.'",
    quest_acc_opt_leave = "[Leave]",

    exit_friendly_reply = "'Safe travels. Remember: let it crash!'"
  }
}
