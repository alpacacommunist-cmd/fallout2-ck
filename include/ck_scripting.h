#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

#include <string>

namespace ck {
	void on_map_enter();
}

namespace fallout {
	void tileWindowRefresh();
}

void ck_call_hook(const char* name);
void ck_call_hook_int(const char* name, int arg);

void ck_reload_mods();
void ck_scripting_init();
void ck_scripting_exit();

void ck_scripting_on_game_start();
void ck_scripting_on_engine_ready();
void ck_scripting_on_before_game_load();
void ck_scripting_on_game_loaded();
void ck_on_scripts_reset();
// state
void ck_scripting_on_game_save(const char* path);
void ck_scripting_on_game_state_load(const char* path);

int ck_get_config_int(const char* key, int default_value);

void ck_scripting_register_location(
		const std::string& name,
		const std::string& mapFile,
		const std::string& music,
		int worldX, int worldY,
		const std::string& size,
		int entranceX, int entranceY, int entranceTile
);

#endif // CK_SCRIPTING_H
