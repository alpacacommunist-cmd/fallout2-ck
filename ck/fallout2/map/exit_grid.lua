local ffi = require("ffi")

local C = ffi.C

local exit_grid = {}

exit_grid.first_pid = C.ck_proto_first_exit_grid_pid
exit_grid.last_pid  = C.ck_proto_last_exit_grid_pid

exit_grid.styles = {
    FIRST = exit_grid.first_pid(),
    LAST  = exit_grid.last_pid(),

    STYLE_0 = exit_grid.first_pid() + 0,
    STYLE_1 = exit_grid.first_pid() + 1,
    STYLE_2 = exit_grid.first_pid() + 2,
    STYLE_3 = exit_grid.first_pid() + 3,
    STYLE_4 = exit_grid.first_pid() + 4,
    STYLE_5 = exit_grid.first_pid() + 5,
    STYLE_6 = exit_grid.first_pid() + 6,
    STYLE_7 = exit_grid.first_pid() + 7
}

exit_grid.create_in_rect  = C.ck_landscape_create_exit_grid_in_rect
exit_grid.destroy_in_rect = C.ck_landscape_destroy_exit_grid_in_rect

function exit_grid.spawn_in_rect(t1, t2, t3, t4, config)
    -- config : { map = 10, tile = 15000, elevation = 0, rotation = 1, style = 0 }

    local grid_data = ffi.new("CKExitGridData")
    grid_data.target_map       = config.map
    grid_data.target_tile      = config.tile
    grid_data.target_elevation = config.elevation or 0
    grid_data.target_rotation  = config.rotation or 0

    local style_offset = config.style or 0
    local final_pid    = exit_grid.styles.FIRST + style_offset

    exit_grid.create_in_rect(t1, t2, t3, t4, final_pid, grid_data)
end

return exit_grid
