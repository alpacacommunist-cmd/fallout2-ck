-- --- File: ck/fallout2/items.lua ---
local ffi = require("ffi")

local items = {}

items.add   = ffi.C.ck_inventory_add
items.count = ffi.C.ck_inventory_count

items.PID_LEATHER_ARMOR = 1
items.PID_METAL_ARMOR   = 2
items.PID_POWER_ARMOR   = 3
items.PID_KNIFE         = 4
items.PID_CLUB          = 5
items.PID_BOTTLE_CAPS   = 41
items.PID_STIMPAK       = 40

return items
