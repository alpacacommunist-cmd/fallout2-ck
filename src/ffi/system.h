#pragma once

#include "ck_api.h"

CK_API bool ck_system_add_mod(const char* mod_id);
CK_API bool ck_system_drop_mod(const char* mod_id);
CK_API bool ck_system_set_mod_context(const char* mod_id);
CK_API void ck_system_emit_for_mod(const char* mod_id, const char* event_name);
CK_API const char* ck_system_current_mod_id();
