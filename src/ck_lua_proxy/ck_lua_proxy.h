#ifndef CK_LUA_PROXY_H
#define CK_LUA_PROXY_H

#include <string>

void ck_lua_proxy_init();
void ck_lua_proxy_shutdown();

namespace ck::proxy {
    extern int emit_for_mod;
    extern int on_map_update;
    extern int on_proc;
    extern int clear_tracked_objects;
    extern int clear_registry;
    extern int load_and_init_mod;
    extern int get_state_tile;
    extern int state_sync_load;
    extern int state_sync_save;

	bool is_ready();
	void execute_map_update(int ticks);
	void execute_emit_for_mod(const std::string& mod_id, const std::string& event_name);

	int execute_get_state_tile(int map_id, const std::string& lua_tag);

	void execute_sync_load();
	void execute_sync_save();
}

#endif
