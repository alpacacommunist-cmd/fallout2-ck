#pragma once

#include "ck_api.h"

CK_API void ck_monitor_print_message(const char *message);
CK_API void ck_sound_play_sfx(const char *name);
CK_API bool ck_object_float_msg(void *ptr, const char *text, int msg_type = 1);
CK_API bool ck_in_combat();
CK_API bool ck_game_is_loading();
CK_API int ck_loading_map_id();
CK_API int ck_current_map_id();
CK_API int ck_current_elevation();
