-- mods/username/test.lua
-- temp launch point

-- load system modules and expose them to engine
require('fallout2.events')
require('fallout2.config')

-- this will load registered mods
require('fallout2.loader')



-- imitate mods load, goes to engine eventually
ckInitializeMods()

local log = require('fallout2.log')
function ckOnDayPassed()
    log.print(
        "Another day passed."
    )
end
