#ifndef CK_HOOKS_H
#define CK_HOOKS_H

namespace ck::hooks::system {
    void init(int argc, char** argv);

    void game_start();
    void engine_ready();
    void scripts_reset();

    void before_game_save();
    void game_saved(const char* path);

    void before_game_load(const char* path);
    void game_loaded();

    void worldmap_open();

    void exit();
}

#endif
