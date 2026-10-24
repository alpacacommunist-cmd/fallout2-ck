#ifndef CK_PUBLIC_HOOKS_H
#define CK_PUBLIC_HOOKS_H

namespace fallout {
    struct Object;
}

namespace ck::hooks::system {
    void init(int argc, char** argv);

    void game_start();
    void engine_ready();
    void scripts_reset();

    void before_game_save();
    void game_saved(const char* path);

    void before_game_load(const char* path);
    void game_loaded();

    void exit();
}

namespace ck::hooks::gameplay {
    void worldmap_open();
    void before_map_load();
    void map_enter();
    void object_destroyed(fallout::Object* object);
}

namespace ck::mods {
    bool reload();
}

#endif
