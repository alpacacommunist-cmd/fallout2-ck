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

    void on_before_game_save();
    void on_game_save(const char* path);
    void on_before_game_load(const char* path);
    void on_game_loaded();
}

bool ck_reloading_mods();
void ck_reload_mods();
void ck_scripting_init(int argc, char** argv);
void ck_scripting_exit();

void ck_scripting_on_object_destroyed(fallout::Object* object);

void ck_scripting_on_game_start();
void ck_scripting_on_engine_ready();
void ck_scripting_on_before_game_load(const char* path);
void ck_scripting_on_game_loaded();
void ck_on_scripts_reset();
// state
void ck_scripting_on_before_game_save();
void ck_scripting_on_game_save(const char* path);

unsigned int ck_current_combat_state();

CK_API void ck_registry_destroy_objects_for_mod(const char* target_mod_id);
CK_API void ck_scripting_load_game_slot(int slot);

CK_API const char* ck_testing_get_current_suite();
CK_API void        ck_testing_set_current_suite(const char* name);

CK_API void ck_scripting_monitor_print_message(const char* message);
CK_API void ck_sound_play_sfx(const char* name);
CK_API bool ck_object_float_msg(void* ptr, const char* text, int msg_type = 1);
CK_API bool ck_in_combat();
CK_API bool ck_mods_reload_in_progress();

#endif // CK_SCRIPTING_H
