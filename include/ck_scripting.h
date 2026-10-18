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

CK_API void ck_mods_clear_resources_for_mod(const char* mod_id);

#endif // CK_SCRIPTING_H
