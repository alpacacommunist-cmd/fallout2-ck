#ifndef CK_EVENTS_H
#define CK_EVENTS_H

#include <string>

namespace fallout {
    struct Object;
}

namespace ck::events::system {
    void init(int argc, char** argv);

    void game_start();
    void engine_ready();
    void scripts_reset();

    void before_game_save();
    void game_saved(const char* path);

    void before_game_load(const char* path);
    void game_loaded();

    void worldmap_open();

    void object_destroyed(fallout::Object* object);
    void exit();

    // Tests helpers
    std::string& current_test_suite_name();
    void set_test_suite(const char* name);
}

#endif
