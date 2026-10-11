#pragma once

#include "ck_api.h"

CK_API bool ck_system_add_mod(const char* mod_id);
CK_API bool ck_system_drop_mod(const char* mod_id);
CK_API bool ck_system_set_mod_context(const char* mod_id);
CK_API void ck_system_emit_for_mod(const char* mod_id, const char* event_name);
CK_API const char* ck_system_current_mod_id();
CK_API bool ck_system_reloading_mods();
// system mod_id (for global procs etc)
CK_API const char* ck_sytem_mod_id();

// Log
CK_API void ck_system_log(int level, int source, int style, const char* context, const char* message);
