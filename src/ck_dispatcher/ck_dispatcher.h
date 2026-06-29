#ifndef CK_DISPATCHER_H
#define CK_DISPATCHER_H

#include "ck_api.h"
#include "object/ck_object_registry.h"

extern "C" {
#include "../../src/vendor/luajit/src/lua.h"
#include "../../src/vendor/luajit/src/lualib.h"
#include "../../src/vendor/luajit/src/lauxlib.h"
}

#include <algorithm>
#include <string>
#include <vector>

void ck_dispatcher_init(lua_State* L);
void ck_dispatcher_shutdown();

void ck_dispatcher_on_game_start();
void ck_dispatcher_on_game_loaded();

void ck_dispatcher_on_day_passed();
void ck_dispatcher_on_time_advance(int hours, int minutes);

void ck_dispatcher_on_map_update(int ticks);
bool ck_dispatcher_on_proc(int lua_id, int proc_id, const char* object_mod_id);

int ck_dispatcher_get_state_tile(int map_id, const char* lua_tag);

CK_API bool ck_dispatcher_load_mod(const char* mod_id);
CK_API const char* ck_get_current_mod_id();
CK_API void ck_dispatcher_emit_for_mod(const char* mod_id, const char* event_name);

#endif
