#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

#include <string>
#include "ck_api.h"

extern "C" bool ck_in_combat();

namespace ck {
	void on_map_enter();
	void reset_dummy_script();
}

namespace fallout {
    struct Object;
	extern Object* gDude;

	void tileWindowRefresh();
	void animationStop();
	void displayMonitorAddMessage(const char* str);
}

void ck_reload_mods();
void ck_scripting_init();
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

int ck_get_config_int(const char* key, int default_value);

void ck_scripting_register_location(
		const std::string& name,
		const std::string& mapFile,
		const std::string& music,
		int worldX, int worldY,
		const std::string& size,
		int entranceX, int entranceY, int entranceTile
);


CK_API void ck_registry_destroy_objects_for_mod(const char* target_mod_id);
CK_API void ck_registry_clear();

#endif // CK_SCRIPTING_H
