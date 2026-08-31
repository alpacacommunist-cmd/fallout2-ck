#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

#include "ck_api.h"

namespace fallout {
    struct Object;
    struct Rect;

	int ck_load_game_slot(int slot);
}

namespace ck::common {
    unsigned int current_combat_state();
    bool currently_in_combat();

    const char* system_mod_id();
    const char* current_mod_id();
    bool reloading_mods();

    void clear_lua_registries();
}

void ck_reload_mods();
void ck_set_language();
void ck_print_monitor_message(const char* message);

namespace ck::events {
    void init(int argc, char** argv);

    void game_start();
    void engine_ready();
    void scripts_reset();

    void before_game_save();
    void game_saved(const char* path);

    void before_game_load(const char* path);
    void game_loaded();

    void object_destroyed(fallout::Object* object);
    void exit();
}

CK_API void ck_load_game_slot(int slot);

CK_API const char* ck_testing_get_current_suite();
CK_API void        ck_testing_set_current_suite(const char* name);

CK_API void ck_monitor_print_message(const char* message);
CK_API void ck_sound_play_sfx(const char* name);
CK_API bool ck_object_float_msg(void* ptr, const char* text, int msg_type = 1);
CK_API bool ck_in_combat();
CK_API bool ck_mods_reload_in_progress();
CK_API const char* ck_mods_system_id();

#endif // CK_SCRIPTING_H
