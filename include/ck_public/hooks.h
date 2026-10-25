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
    // worldmap.cc
    void worldmap_open();
    // map.cc
    void before_map_load();
    void map_enter();
    // object.cc
    void object_destroyed(fallout::Object* object);
    // script.cc (game time), proto_instance.cc, pipboy.cc, skill.cc
	void time_advance(int hours, int minutes);
    void after_rest(int hours, int minutes);
	void day_passed();
}

namespace ck::mods {
    bool reload();
}

#endif
