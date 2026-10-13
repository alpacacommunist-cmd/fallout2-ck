#ifndef CK_GAMEPLAY_COMMON_H
#define CK_GAMEPLAY_COMMON_H

namespace ck::common {
    unsigned int current_combat_state();
    bool in_combat();

    int current_elevation();
    int current_map_id();

    int square(int elevation, int square_tile);
    bool hex_grid_tile_is_valid(int tile);
    bool square_grid_tile_is_valid(int tile);

    bool game_is_loading();
    int loading_map_id();

    void play_sfx(const char *name);
    void print_monitor_message(const char* message);
    bool object_float_msg(void* ptr, const char* text, int msg_type);
}

#endif
