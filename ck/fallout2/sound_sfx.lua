local ffi = require('ffi')
local log = ck.log.new('sound_sfx.lua')

local sfx = {}

sfx.presets = {
  click       = "ib1p1xx1",
  release     = "ib1lu1x1",
  geiger      = "GEIGER",
  zipper      = "ZIPPER",
  inventory   = "inv_open",
  scorp_death = "scorpdie",
}

function sfx.play(sound_name)
  local file_name = sfx.presets[sound_name] or sound_name

  file_name = string.upper(file_name)

  ffi.C.ck_sound_play_sfx(file_name)
end

return sfx
