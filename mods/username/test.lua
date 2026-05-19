-- mods/username/test.lua

-- checks if lua even works
print("hello from lua to linux terminal (still works!)")


-- checks ck api
local log = require('fallout2.log')
log.print("HELLO FROM MOD VIA REQUIRE!!!")

-- this goes into fallout2.events in the future
function ckOnGameStart()
    print("[Lua] Game is Ready! Triggering onGameStart hook...")
    
    -- Try printing text into interface monitor!
    log.print("STATION CK: ONLINE AND READY TO MOD!")
    log.print("You see: A Giant Ant! Oh wait, actually it's LuaJit!")
end

