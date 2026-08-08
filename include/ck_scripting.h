#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

#include "ck_api.h"

extern "C" bool ck_in_combat();

namespace ck {
	void on_map_enter();
}

namespace fallout {
    struct Object;
	extern Object* gDude;

	void tileWindowRefresh();
	void animationStop();
	void displayMonitorAddMessage(const char* str);

	void mainMenuWindowFree();

	int  ck_load_game_slot(int slot);
}


void ck_map_clear_camera_borders();

void ck_reload_mods();
void ck_scripting_init(int argc, char** argv);
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

CK_API void ck_registry_destroy_objects_for_mod(const char* target_mod_id);
CK_API void ck_scripting_load_game_slot(int slot);

CK_API const char* ck_testing_get_current_suite();
CK_API void ck_testing_set_current_suite(const char* name);
CK_API void ck_scripting_monitor_print_message(const char* message);

#endif // CK_SCRIPTING_H
