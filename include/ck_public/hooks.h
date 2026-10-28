#ifndef CK_PUBLIC_HOOKS_H
#define CK_PUBLIC_HOOKS_H

namespace fallout {
    struct Object;
    struct Script;
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
    // scripts.cc (game time), proto_instance.cc, pipboy.cc, skill.cc
	void time_advance(int hours, int minutes);
    void after_rest(int hours, int minutes);
	void day_passed();
    // scripts.cc
	void map_update(unsigned int ticks);
    bool proc(fallout::Script* script, int proc);
    void proto_script_requested(fallout::Script* script);
    // game_dialog.cc
    void last_dialog_choice(int option_index);
    // object.cc
    void object_destroyed(fallout::Object* object);
    // item.cc
    bool items_identical_check(fallout::Object* item1, fallout::Object* item2);
}

namespace ck::mods {
    bool reload();
}

#endif
