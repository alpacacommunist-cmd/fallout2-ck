#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

#include "ck_api.h"

namespace fallout {
    struct Rect;
}

namespace ck::common {
    unsigned int current_combat_state();
    bool in_combat();

    int current_map_id();
    int current_elevation();

    const char* system_mod_id();

    bool game_is_loading();
    int  loading_map_id();

    void clear_lua_registries();
    void lua_map_exit();

    void print_monitor_message(const char* message);
}

void ck_reload_mods();
void ck_clear_mod_resources(const char* mod_id);
void ck_set_language();
void ck_print_monitor_message(const char* message);

CK_API void ck_load_game_slot(int slot);

CK_API void ck_monitor_print_message(const char* message);
CK_API void ck_sound_play_sfx(const char* name);
CK_API bool ck_object_float_msg(void* ptr, const char* text, int msg_type = 1);
CK_API bool ck_in_combat();
CK_API bool ck_mods_reload_in_progress();
CK_API bool ck_game_is_loading();
CK_API int  ck_loading_map_id();
CK_API int  ck_current_map_id();
CK_API int ck_current_elevation();
CK_API const char* ck_mods_system_id();
CK_API void ck_mods_clear_resources_for_mod(const char* mod_id);

#endif // CK_SCRIPTING_H
