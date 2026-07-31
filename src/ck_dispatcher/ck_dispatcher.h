#ifndef CK_DISPATCHER_H
#define CK_DISPATCHER_H

#include "ck_api.h"

void ck_dispatcher_on_game_start();
void ck_dispatcher_on_game_loaded();

void ck_dispatcher_on_day_passed();
void ck_dispatcher_on_time_advance(int hours, int minutes);
void ck_dispatcher_on_skill_used(int skill, int success_count, int bonus);

void ck_dispatcher_on_map_update(int ticks);
bool ck_dispatcher_on_proc(int lua_id, int proc_id, int fixed_param, const char* object_mod_id);

CK_API bool ck_dispatcher_load_mod(const char* mod_id);
CK_API const char* ck_get_current_mod_id();
CK_API void ck_dispatcher_emit_for_mod(const char* mod_id, const char* event_name);

#endif
